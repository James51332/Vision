#include "ShaderCompiler.h"

#include <SDL.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

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
    default: break;
  }

  return EShLangVertex;
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

  // Make sure this pointer is retained through the entire compilation.
  const char* sourceStr = shaderSource.Source.c_str();
  shader.setStrings(&sourceStr, 1);
  {
    shader.setEnvInput(glslang::EShSourceGlsl, language, glslang::EShClientOpenGL, 450);
    shader.setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
    shader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_1);

    shader.setAutoMapBindings(true);
    shader.setAutoMapLocations(true);
  }

  shader.setEntryPoint("main");

  // Parse the shader
  if (!shader.parse(GetDefaultResources(), 410, true,
                    static_cast<EShMessages>(EShMsgDefault | EShMsgDebugInfo | EShMsgSpvRules)))
  {
    std::cout << "Failed to compile shader: " << shaderSource.Name << std::endl;
    std::cout << shader.getInfoLog() << std::endl;
    std::cout << shader.getInfoDebugLog() << std::endl;
    return {};
  }

  // TODO: Generate based on Vision configuration.
  glslang::SpvOptions options;
  options.disableOptimizer = false;
  options.generateDebugInfo = true;
  options.optimizeSize = true;

  // Finalize and Compile
  glslang::GlslangToSpv(*shader.getIntermediate(), spirv, &logger, &options);
  glslang::FinalizeProcess();

  return {shaderSource.Stage, shaderSource.Name, std::move(spirv)};
}

std::vector<ShaderSPIRV> ShaderCompiler::CompileFile(const std::string& filePath, bool canCache)
{
  std::vector<ShaderSPIRV> shaderSPIRVs;
  CompileFile(filePath, shaderSPIRVs, canCache);
  return shaderSPIRVs;
}

static std::string ShaderStageToSuffix(ShaderStage stage)
{
  switch (stage)
  {
    case ShaderStage::Vertex: return "vert";
    case ShaderStage::Pixel: return "pixel";
    case ShaderStage::Geometry: return "geometry";
    case ShaderStage::Compute: return "comp";
    case ShaderStage::Domain: return "domain";
    case ShaderStage::Hull: return "hull";
  }

  throw new std::runtime_error("Illegal shader stage to convert to file suffix!");
}

static ShaderStage SuffixToShaderStage(std::string suffix)
{
  if (suffix == "vert")
    return ShaderStage::Vertex;
  if (suffix == "pixel")
    return ShaderStage::Pixel;
  if (suffix == "geometry")
    return ShaderStage::Geometry;
  if (suffix == "comp")
    return ShaderStage::Compute;
  if (suffix == "domain")
    return ShaderStage::Domain;
  if (suffix == "hull")
    return ShaderStage::Hull;

  throw new std::runtime_error("Unknown shader stage file extension!");
}

void ShaderCompiler::CompileFile(const std::string& filePath, std::vector<ShaderSPIRV>& destination,
                                 bool canCache)
{
  SDL_assert(std::filesystem::exists(filePath));

  // Each file that we try to compile gets a folder in the cache that holds all spirv files.
  const std::string cacheFolder = "cache";
  std::string cachePath = cacheFolder + "/" + filePath;

  // If the cache path exists, we'll go ahead and load from it.
  if (canCache && std::filesystem::exists(cachePath) && std::filesystem::is_directory(cachePath))
  {
    auto cacheTime = std::filesystem::last_write_time(cachePath);
    auto updateTime = std::filesystem::last_write_time(filePath);

    if (cacheTime > updateTime)
    {
      for (auto& entry : std::filesystem::directory_iterator(cachePath))
      {
        // Each file is a ShaderSPIRV.
        std::ifstream file(entry.path(), std::ios::binary | std::ios::in);

        // Read the file into our vector of data.
        std::uint32_t fileLength = std::filesystem::file_size(entry.path());
        std::vector<uint32_t> data((fileLength + 3) / 4);
        file.read(reinterpret_cast<char*>(data.data()), fileLength);
        file.close();

        // Prepare a shader spirv based on the data.
        std::string filename = entry.path().filename().string();

        // Find the period that splits the name.
        auto index = filename.find(".");
        if (index == std::string::npos)
          std::cout << "Warning: Unable to load shader in cache " << cachePath << std::endl;

        std::string shaderName = filename.substr(0, index);
        ShaderStage shaderStage = SuffixToShaderStage(filename.substr(index + 1));

        destination.push_back({shaderStage, shaderName, data});
      }

      // Once we have loaded all from the cache, we are done.
      return;
    }
  }

  // Here we handle the case that we don't have a valid cache. Let's build one.
  if (canCache && !std::filesystem::exists(cachePath) &&
      !std::filesystem::create_directories(cachePath))
  {
    canCache = false;
    std::cout << "Warning: unable to create shader cache folder" << std::endl;
  }

  // Compile all shaders and add them to the cache, one at a time.
  ShaderParser parser;
  std::vector<ShaderSource> shaderSources = parser.ParseFile(filePath);
  for (auto& source : shaderSources)
  {
    // We can compile our shader and add it to the destination.
    ShaderSPIRV compiled = CompileSource(source);
    destination.push_back(compiled);

    if (!canCache)
      continue;

    // To generate the path, add the cache directory, and a suffix to represent the type.
    std::string fileName =
        cachePath + "/" + compiled.Name + "." + ShaderStageToSuffix(compiled.Stage);
    std::ofstream stream(fileName, std::ios::binary | std::ios::out);

    // Make sure that we can write to this file
    if (!stream.is_open())
    {
      std::cout << "Warning: Unable to cache shader " << fileName << std::endl;
      continue;
    }

    // Clear and write to the directory
    stream.clear();
    stream.write(reinterpret_cast<char*>(compiled.SPIRV.data()), compiled.SPIRV.size() * 4);
    stream.close();
  }
}

std::unordered_map<std::string, ShaderSPIRV>
ShaderCompiler::CompileFileToMap(const std::string& filePath, bool canCache)
{
  // Compile file as normal, then unpack into a map
  std::vector<ShaderSPIRV> shaderSPIRVs = ShaderCompiler::CompileFile(filePath, canCache);
  std::unordered_map<std::string, ShaderSPIRV> map;

  // Then iterate
  for (ShaderSPIRV& spirv : shaderSPIRVs)
  {
    if (map.find(spirv.Name) == map.end())
      map[spirv.Name] = spirv;
    else
      std::cout << "Warning: File (" << filePath
                << ") contains multiple shaders with name: " << spirv.Name << std::endl;
  }

  return map;
}

} // namespace Vision