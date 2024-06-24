#include "ShaderCompiler.h"

#include <iostream>
#include <sstream>
#include <SDL.h>

#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>

#include <SPIRV/GlslangToSpv.h>
#include <spirv_glsl.hpp>

namespace Vision
{

// helper functions
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
  if (type == "compute")
    return ShaderStage::Compute;

  SDL_assert(false);
  return ShaderStage::Vertex;
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
    case ShaderStage::Compute: return EShLangCompute;
  }
}

void ShaderCompiler::GenerateStageMap(ShaderDesc& desc)
{
  // don't override existing stage map if manually enabled.
  SDL_assert(desc.Source == ShaderSource::File);
  std::string rawCode = ReadFile(desc.FilePath);

  // prepare our stage map
  desc.StageMap.clear();
  desc.StageMap = Parse(rawCode);

  // ensure we have a vertex and fragment shader
  SDL_assert(desc.StageMap[ShaderStage::Vertex].size() != 0);
  SDL_assert(desc.StageMap[ShaderStage::Pixel].size() != 0);

  // set the source to be stage map
  desc.Source = ShaderSource::GLSL;
}

void ShaderCompiler::GenerateSPIRVMap(ShaderDesc& desc)
{
  SDL_assert(desc.Source == ShaderSource::GLSL);

  for (auto pair : desc.StageMap)
  {
    std::string& code = pair.second;
    auto stage = pair.first;

    desc.SPIRVMap[stage] = Compile(code, stage);
  }

  // set the new source mode to SPIRV map
  desc.Source = ShaderSource::SPIRV;
}

void ShaderCompiler::LoadSource(ComputePipelineDesc &desc)
{
  SDL_assert(desc.Source == ShaderSource::File);

  desc.GLSL = ReadFile(desc.FilePath);
  desc.Source = ShaderSource::GLSL;
}

void ShaderCompiler::GenerateSPIRV(ComputePipelineDesc &desc)
{
  SDL_assert(desc.Source == ShaderSource::GLSL);

  auto map = Parse(desc.GLSL);
  SDL_assert(map.size() == 1);
  SDL_assert(map[ShaderStage::Compute].size() != 0);

  desc.SPIRV = Compile(map[ShaderStage::Compute], ShaderStage::Compute);
  desc.Source = ShaderSource::SPIRV;
}

std::string ShaderCompiler::ReadFile(const std::string &filePath)
{
  SDL_RWops *file = SDL_RWFromFile(filePath.c_str(), "r+");
  if (!file)
  {
    SDL_Log("Failed to open shader: %s", SDL_GetError());
    return "Error: Invalid File!";
  }

  size_t size = SDL_RWsize(file);
  std::string buffer(size, ' ');
  SDL_RWread(file, &buffer[0], size);
  SDL_RWclose(file);

  return std::move(buffer);
}

std::unordered_map<ShaderStage, std::string> ShaderCompiler::Parse(std::string& source)
{
  std::unordered_map<ShaderStage, std::string> sources;

  // System from TheCherno/Hazel
  const char *typeToken = "#type";
  size_t typeTokenLength = strlen(typeToken);
  std::size_t pos = source.find(typeToken, 0);
  while (pos != std::string::npos)
  {
    size_t eol = source.find_first_of("\r\n", pos); // End of shader type declaration line
    SDL_assert(eol != std::string::npos);
    size_t begin = pos + typeTokenLength + 1; // Start of shader type name (after "#type " keyword)
    std::string type = source.substr(begin, eol - begin);

    size_t nextLinePos = source.find_first_not_of("\r\n", eol); // Start of shader code after shader type declaration line
    SDL_assert(nextLinePos != std::string::npos);
    pos = source.find(typeToken, nextLinePos); // Start of next shader type declaration line

    sources[ShaderStageFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
  }

  return std::move(sources);
}

std::vector<uint32_t> ShaderCompiler::Compile(std::string &source, ShaderStage stage)
{
  // basic data
  std::vector<uint32_t> spirv;
  EShLanguage language = ShaderStageToEShLanguage(stage);
  spv::SpvBuildLogger logger;

  // configure glslang compiler
  glslang::InitializeProcess();

  // create and configure shader
  glslang::TShader shader(language);
  {
    const char* sourceStr = source.c_str();
    shader.setStrings(&sourceStr, 1);

    shader.setEnvInput(glslang::EShSourceGlsl, language, glslang::EShClientOpenGL, 450);
    shader.setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
    shader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_1);

    shader.setAutoMapBindings(true);
    shader.setAutoMapLocations(true);
  }

  // parse the shader
  if (!shader.parse(GetDefaultResources(), 100, false, static_cast<EShMessages>(EShMsgDefault | EShMsgDebugInfo | EShMsgSpvRules)))
  {
    std::cout << "Failed to compile shader:" << std::endl;
    std::cout << shader.getInfoLog() << std::endl;
    return std::vector<uint32_t>(32, 0); // attempting to convert to spirv will crash
  }

  // TODO: configure this based on build mode
  glslang::SpvOptions options;
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
    return { 0 };
  }

  // finalize and compile
  glslang::GlslangToSpv(*program.getIntermediate(language), spirv, &logger, &options);
  glslang::FinalizeProcess();

  return std::move(spirv);
}

}