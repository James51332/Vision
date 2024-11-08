#include "MetalBuffer.h"

#include <Metal/MTLResource.hpp>
#include <SDL.h>
#include <iostream>

#include "MetalDevice.h"

namespace Vision
{

MetalBuffer::MetalBuffer(MetalDevice* device, const BufferDesc& desc)
    : type(desc.Type), size(desc.Size), name(desc.DebugName),
      isDynamicBuffer(desc.Usage == BufferUsage::Dynamic)
{
  MTL::Device* gpuDevice = device->GetDevice();

  // Dynamic buffers are triple buffered so that we can update them while they are being used by the
  // GPU. Static buffers do not need to be updated, so we can store the data in a single buffer.
  if (desc.Usage == BufferUsage::Static)
    buffers = std::vector<MTL::Buffer*>(1, nullptr);
  else
    buffers = std::vector<MTL::Buffer*>(device->GetMaxFramesInFlight(), nullptr);

  // Create each buffer.
  for (int i = 0; i < buffers.size(); ++i)
  {
    MTL::Buffer* buffer;

    if (desc.Data)
      buffer = gpuDevice->newBuffer(desc.Data, desc.Size, MTL::ResourceStorageModeShared);
    else
      buffer = gpuDevice->newBuffer(desc.Size, MTL::ResourceStorageModeShared);

    std::string name = desc.DebugName;
    if (buffers.size() > 1)
      name = name + " (" + std::to_string(i) + ")";
    buffer->setLabel(NS::String::alloc()->init(desc.DebugName.c_str(), NS::UTF8StringEncoding));

    // Add our buffer to the array.
    buffers[i] = buffer;
  }
}

MetalBuffer::~MetalBuffer()
{
  for (MTL::Buffer* buffer : buffers)
    buffer->release();
}

void MetalBuffer::SetData(MetalDevice* device, std::size_t s, void* data, std::size_t offset)
{
  SDL_assert(offset + s <= size);

  // Transition to the buffer for this frame-in-flight.
  if (isDynamicBuffer)
    bufferIndex = device->GetInFlightFrame();

  char* bufferAddr = static_cast<char*>(buffers[device->GetInFlightFrame()]->contents());
  std::memcpy(bufferAddr + offset, data, s);
}

void MetalBuffer::Reset(MetalDevice* device, std::size_t newSize)
{
  size = newSize;

  for (MTL::Buffer* buffer : buffers)
  {
    NS::String* name = buffer->label()->copy();
    buffer->release();

    buffer = device->GetDevice()->newBuffer(size, MTL::ResourceStorageModeShared);
    buffer->setLabel(name);
  }
}

} // namespace Vision
