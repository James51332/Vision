#pragma once

#include <Metal/Metal.hpp>
#include <string>

#include "renderer/primitive/Buffer.h"

namespace Vision
{

class MetalBuffer
{
public:
  MetalBuffer(MTL::Device* device, const BufferDesc& desc);
  ~MetalBuffer();

  void SetData(std::size_t size, void* data);
  void Reset(MTL::Device* device, std::size_t size);

public:
  MTL::Buffer* buffer = nullptr;

  BufferType type;
  std::size_t size;
  std::string name;
};

}