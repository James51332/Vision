#include "ShaderCompiler.h"

#include <iostream>
#include <sstream>
#include <SDL.h>

#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <spirv_glsl.hpp>

namespace Vision
{

static const char* ShaderStageToString(ShaderStage type)
{
  switch (type)
  {
    case ShaderStage::Vertex: return "vertex";
    case ShaderStage::Pixel: return "pixel";
    case ShaderStage::Domain: return "domain";
    case ShaderStage::Hull: return "hull";
    case ShaderStage::Geometry: return "geometry";
    default:
      return "unknown";
  }
}

static ShaderStage ShaderStageFromString(const std::string &type)
{
  if (type == "vertex")
    return ShaderStage::Vertex;
  if (type == "fragment" || type == "pixel")
    return ShaderStage::Pixel;
  if (type == "tesselation_control" || type == "tcs" || type == "hull")
    return ShaderStage::Hull;
  if (type == "tesselation_evaluation" || type == "tes" || type == "domain")
    return ShaderStage::Domain;
  if (type == "geometry")
    return ShaderStage::Geometry;

  SDL_assert(false);
  return ShaderStage::Vertex;
}

void ShaderCompiler::GenerateStageMap(ShaderDesc& desc)
{
  // don't override existing stage map if manually enabled.
  SDL_assert(desc.Source == ShaderSource::File);

  // read the file from disc.
  SDL_RWops *shader = SDL_RWFromFile(desc.FilePath.c_str(), "r+");
  if (!shader)
  {
    SDL_Log("Failed to open shader: %s", SDL_GetError());
  }

  size_t size = SDL_RWsize(shader);
  std::string buffer(size, ' ');
  SDL_RWread(shader, &buffer[0], size);

  // prepare our stage map
  desc.StageMap.clear();

  // System from TheCherno/Hazel
  const char* typeToken = "#type";
  size_t typeTokenLength = strlen(typeToken);
  std::size_t pos = buffer.find(typeToken, 0);
  while (pos != std::string::npos)
  {
    size_t eol = buffer.find_first_of("\r\n", pos); // End of shader type declaration line
    SDL_assert(eol != std::string::npos);
    size_t begin = pos + typeTokenLength + 1; // Start of shader type name (after "#type " keyword)
    std::string type = buffer.substr(begin, eol - begin);

    size_t nextLinePos = buffer.find_first_not_of("\r\n", eol); // Start of shader code after shader type declaration line
    SDL_assert(nextLinePos != std::string::npos);
    pos = buffer.find(typeToken, nextLinePos); // Start of next shader type declaration line

    desc.StageMap[ShaderStageFromString(type)] = (pos == std::string::npos) ? buffer.substr(nextLinePos) : buffer.substr(nextLinePos, pos - nextLinePos);
  }

  // ensure we have a vertex and fragment shader
  SDL_assert(desc.StageMap[ShaderStage::Vertex].size() != 0);
  SDL_assert(desc.StageMap[ShaderStage::Pixel].size() != 0);

  // set the source to be stage map
  desc.Source = ShaderSource::StageMap;
}

static EShLanguage ShaderStageToEShLanguage(ShaderStage stage)
{
  switch (stage)
  {
    case ShaderStage::Vertex: return EShLangVertex;
    case ShaderStage::Pixel: return EShLangFragment;
    case ShaderStage::Hull: return EShLangTessControl;
    case ShaderStage::Domain: return EShLangTessEvaluation;
    case ShaderStage::Geometry: return EShLangGeometry;
  }
}

void ShaderCompiler::GenerateSPIRVMap(ShaderDesc& desc)
{
  SDL_assert(desc.Source == ShaderSource::StageMap);

  for (auto pair : desc.StageMap)
  {
    auto stage = pair.first;
    std::string text = pair.second;
    const char* textPtr = text.c_str();

    glslang::InitializeProcess();

    EShLanguage language = ShaderStageToEShLanguage(stage);
    glslang::TShader shader(language);

    shader.setStrings(&textPtr, 1);
    shader.setEnvInput(glslang::EShSourceGlsl, language, glslang::EShClientOpenGL, 410);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
    shader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_1);

    shader.setAutoMapBindings(true);
    shader.setAutoMapLocations(true);

    if (!shader.parse(GetDefaultResources(), 100, false, static_cast<EShMessages>(EShMsgDefault | EShMsgDebugInfo | EShMsgSpvRules)))
    {
      std::cout << "Failed to compile shader:" << std::endl;
      std::cout << shader.getInfoLog() << std::endl;
      return; // attempting to convert to spirv will crash
    }

    std::vector<uint32_t> data;
    spv::SpvBuildLogger logger;
    glslang::SpvOptions options;

    // TODO: configure this based on build mode
    options.disableOptimizer = true;
    options.generateDebugInfo = true;
    options.optimizeSize = false;

    // each program only has one shader
    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(static_cast<EShMessages>(EShMsgDefault | EShMsgDebugInfo | EShMsgSpvRules)) || !program.mapIO())
    {
      std::cout << "Failed to link program:" << std::endl;
      std::cout << program.getInfoLog() << std::endl;
    }

    glslang::GlslangToSpv(*program.getIntermediate(language), data, &logger, &options);
    desc.SPIRVMap[stage] = std::move(data);

    glslang::FinalizeProcess();
  }

  // set the new source mode to SPIRV map
  desc.Source = ShaderSource::SPIRV;
}

}