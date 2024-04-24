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

void ShaderCompiler::GenerateSPIRVMap(ShaderDesc& desc)
{
  // TODO: I'm going to write this in the next commit.

  // SDL_assert(desc.Source == ShaderSource::StageMap);
  // if (desc.FilePath != "resources/skyShader.glsl") return;

  // for (auto pair : desc.StageMap)
  // {
  //   ShaderStage stage = pair.first;
  //   std::string& text = pair.second;

  //   if (stage != ShaderStage::Vertex) continue;
  //   std::cout << "Running Test on Vertex Shader!" << std::endl;

  //   glslang::InitializeProcess();

  //   // First compile the shader
  //   glslang::TShader shader(EShLangVertex);
  //   const char* str = text.c_str();
  //   shader.setStrings(&str, 1);
  //   shader.setEnvInput(glslang::EShSourceGlsl, EShLangVertex, glslang::EShClientOpenGL, 410);
  //   shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
  //   shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_1);

  //   // Old GLSL compiler expects us to have locations for uniforms
  //   shader.setAutoMapBindings(true);
  //   shader.setAutoMapLocations(true);

  //   if (!shader.parse(GetDefaultResources(), 100, false, static_cast<EShMessages>(EShMsgDefault | EShMsgDebugInfo | EShMsgSpvRules)))
  //   {
  //     std::cout << "failed to compile shader" << std::endl;
  //     std::cout << shader.getInfoLog() << std::endl;
  //   }

  //   std::vector<uint32_t> data;
  //   spv::SpvBuildLogger logger;
  //   glslang::SpvOptions options;
  //   options.disableOptimizer = true;
  //   options.generateDebugInfo = true;
  //   options.optimizeSize = false;

  //   // each program only has one shader
  //   glslang::TProgram program;
  //   program.addShader(&shader);

  //   if (!program.link(static_cast<EShMessages>(EShMsgDefault | EShMsgDebugInfo | EShMsgSpvRules)) || !program.mapIO())
  //   {
  //     std::cout << "failed to link program" << std::endl;
  //     std::cout << program.getInfoLog() << std::endl;
  //   }

  //   program.buildReflection();

  //   glslang::GlslangToSpv(*program.getIntermediate(EShLangVertex), data, &logger, &options);
  //   std::cout << logger.getAllMessages() << std::endl;
  //   glslang::FinalizeProcess();

  //   // Then decompile
  //   spirv_cross::CompilerGLSL comp(data);
  //   spirv_cross::CompilerGLSL::Options opt;
  //   opt.version = 410;
  //   opt.es = false;
  //   opt.enable_420pack_extension = false;
  //   opt.emit_push_constant_as_uniform_buffer = true;
  //   comp.set_common_options(opt);
  //   std::string output = comp.compile();

  //   std::cout << output << std::endl;
  // }
}

}