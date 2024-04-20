#pragma once

#include "primitive/Shader.h"

namespace Vision
{

// Here's the move for this. The render device will own a shader compiler.
// Whenever we need to complie a shader, we'll check if we need to generate a
// stage map. This method is then called, and then we pass to the APIDevice.
class ShaderCompiler
{
public:
  void GenerateStageMap(ShaderDesc& desc);
};

}