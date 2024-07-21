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

ShaderSPIRV ShaderCompiler::CompileSource(const ShaderSource& shaderSource)
{
  std::vector<uint32_t> spirv;
  EShLanguage language = ShaderStageToEShLanguage(shaderSource.Stage);
  spv::SpvBuildLogger logger;

  // Configure glslang compiler
  glslang::InitializeProcess();

  // Create and configure shader
  glslang::TShader shader(language);
  {
    const char* sourceStr = shaderSource.Source.c_str();
    shader.setStrings(&sourceStr, 1);

    shader.setEnvInput(glslang::EShSourceGlsl, language, glslang::EShClientOpenGL, 450);
    shader.setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
    shader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_1);

    shader.setAutoMapBindings(true);
    shader.setAutoMapLocations(true);
  }

  // Parse the shader
  if (!shader.parse(GetDefaultResources(), 100, false, static_cast<EShMessages>(EShMsgDefault | EShMsgDebugInfo | EShMsgSpvRules)))
  {
    std::cout << "Failed to compile shader:" << std::endl;
    std::cout << shader.getInfoLog() << std::endl;
    return {};
  }

  // TODO: configure this based on build mode
  glslang::SpvOptions options;
  options.disableOptimizer = true;
  options.generateDebugInfo = true;
  options.optimizeSize = false;

  // Each program only has one shader
  glslang::TProgram program;
  program.addShader(&shader);

  if (!program.link(static_cast<EShMessages>(EShMsgDefault | EShMsgDebugInfo | EShMsgSpvRules)) || !program.mapIO())
  {
    std::cout << "Failed to link program:" << std::endl;
    std::cout << program.getInfoLog() << std::endl;
    return {};
  }

  // Finalize and Compile
  glslang::GlslangToSpv(*program.getIntermediate(language), spirv, &logger, &options);
  glslang::FinalizeProcess();

  return { shaderSource.Stage, shaderSource.Name, std::move(spirv) };
}

std::vector<ShaderSPIRV> ShaderCompiler::CompileFile(const std::string& filePath)
{
  ShaderParser parser;
  std::vector<ShaderSource> shaderSources = parser.ParseFile(filePath);

  std::vector<ShaderSPIRV> shaderSPIRVs;
  shaderSPIRVs.reserve(shaderSources.size());
  for (auto& source : shaderSources)
  {
    shaderSPIRVs.push_back(CompileSource(source));
  }
}

}