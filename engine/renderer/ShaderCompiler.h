#pragma once

#include "primitive/Shader.h"

namespace Vision
{

class ShaderCompiler
{
public:
  // Takes a shader with a file path for us to load, parse that file, and produce strings for each stage.
  void GenerateStageMap(ShaderDesc& desc);

  // Takes a set of strings for each shader and generates the SPIRV for them.
  void GenerateSPIRVMap(ShaderDesc& desc);
};

}