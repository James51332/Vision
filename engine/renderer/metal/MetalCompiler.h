#pragma once

#include <Metal/Metal.hpp>

#include "renderer/shader/Shader.h"

namespace Vision
{

class MetalCompiler
{
public:
  MTL::Function* Compile(MTL::Device* device, const ShaderSPIRV& spirv);
};

}