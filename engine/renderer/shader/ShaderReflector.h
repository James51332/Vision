#pragma once

#include <spirv_reflect.hpp>
#include <glm/glm.hpp>
#include <vector>

#include "Shader.h"

namespace Vision
{

// Simple API to retrieve info about a shader from its SPIRV.
// Currently can only retrieve info about uniform and sampled
// image bindings as well thread group size. There are certainly
// far more things that could be queried; however, I'm limiting the
// API to what we currently use in the engine for the time being.
class ShaderReflector
{
public:
  ShaderReflector(const ShaderSPIRV& spirv);

  glm::ivec3 GetThreadgroupSize() const;

  struct UniformBuffer
  {
    std::string Name;
    std::size_t Binding;
  };
  std::vector<UniformBuffer> GetUniformBuffers() const;

  struct SampledImage
  {
    std::string Name;
    std::size_t Binding;
    std::size_t ArraySize;
  };
  std::vector<SampledImage> GetSampledImages() const;

private:
  spirv_cross::CompilerReflection reflector;
};

}