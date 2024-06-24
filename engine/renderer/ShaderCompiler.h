#pragma once

#include "primitive/Shader.h"
#include "primitive/Pipeline.h"

namespace Vision
{

// TODO: It may be nice to create a unified API for this so it's more modularized from the rest of the
// render and compute pipeline. Right now it just does what we need, and that's okay with me.
class ShaderCompiler
{
public:
  // Takes a shader with a file path for us to load, parse that file, and produce strings for each stage.
  void GenerateStageMap(ShaderDesc& desc);

  // Takes a set of strings for each shader and generates the SPIRV for them.
  void GenerateSPIRVMap(ShaderDesc& desc);

  void LoadSource(ComputePipelineDesc& desc);
  void GenerateSPIRV(ComputePipelineDesc& desc);

  std::string ReadFile(const std::string& filePath);
  std::unordered_map<ShaderStage, std::string> Parse(std::string& source);
  std::vector<uint32_t> Compile(std::string& source, ShaderStage stage);
};

}