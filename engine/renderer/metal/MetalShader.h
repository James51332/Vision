#pragma once

#include <Metal/Metal.hpp>

#include "renderer/primitive/Shader.h"

namespace Vision
{

class MetalShader
{
public:
  MetalShader();
  MetalShader(MTL::Device* device, const std::unordered_map<ShaderStage, std::string> &shaders);
  ~MetalShader();

  MTL::Function* GetFunction(ShaderStage stage) { return shaderFunctions.at(stage); }

private:
  std::unordered_map<ShaderStage, MTL::Function*> shaderFunctions;
};

}