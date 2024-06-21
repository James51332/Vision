#pragma once

#include <Metal/Metal.hpp>

#include "renderer/primitive/Shader.h"

namespace Vision
{

class MetalShader
{
public:
  MetalShader();
  MetalShader(MTL::Device* device, const std::unordered_map<ShaderStage, std::string> &shaders, const std::unordered_map<std::string, std::size_t>& slotsMap);
  ~MetalShader();

  MTL::Function* GetFunction(ShaderStage stage) { return shaderFunctions.at(stage); }

  // Our next step is to determine the API for uniform buffers. One is to automatically generate them
  // using reflection. Another idea is to retrieve binding slots for each named uniform. Or we could
  // mandate that the shader and program must manually have the same locations. This is the simplest
  // solution, and what we'll use to begin with. The other factor to consider is the metal requires us
  // to set our stage input buffers, and these must not overlap with uniform buffers. To handle this,
  // we'll fetch all of the slot used by uniform buffers and find the first free slots when we build
  // a pipeline. Then we can cache these slots and use them when we bind. We'll also note that this
  // only applies to vertex buffers. If we need other stages, the only method for now is to pass between
  // stages.
  const std::vector<std::size_t>& GetUniformBufferSlots() const { return uniformSlots; }

  // I'm also going to preemptively add an API that quieries the slot for a given uniform buffer.
  std::size_t GetUniformBufferSlot(const std::string& name) const { return uniformSlotsMap.at(name); }

private:
  std::unordered_map<ShaderStage, MTL::Function*> shaderFunctions;

  // These only apply to vertex
  std::vector<std::size_t> uniformSlots;
  std::unordered_map<std::string, std::size_t> uniformSlotsMap; 
};

}