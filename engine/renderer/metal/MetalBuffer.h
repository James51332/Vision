#pragma once

#include <Metal/Metal.hpp>
#include <string>

#include "renderer/primitive/Buffer.h"

namespace Vision
{

class MetalDevice;

class MetalBuffer
{
public:
  MetalBuffer(MetalDevice* device, const BufferDesc& desc);
  ~MetalBuffer();

  void SetData(MetalDevice* device, std::size_t size, void* data, std::size_t offset);
  void Reset(MetalDevice* device, std::size_t newSize);

  MTL::Buffer* GetActiveBuffer() { return buffers[bufferIndex]; }

private:
  std::vector<MTL::Buffer*> buffers;

  BufferType type;
  std::size_t size;
  std::string name;

  // Since we are using a triple-buffering approach, we track the last buffer this buffer was
  // written to on the CPU. Alternatively, we could explicitly require the user to transition
  // buffers.
  bool isDynamicBuffer;
  std::size_t bufferIndex = 0;
};

} // namespace Vision