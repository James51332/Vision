#include "MetalBuffer.h"

#include <Metal/MTLResource.hpp>
#include <SDL.h>
#include <iostream>

namespace Vision
{

MetalBuffer::MetalBuffer(MTL::Device* device, const BufferDesc& desc)
    : type(desc.Type), size(desc.Size), name(desc.DebugName)
{
  if (desc.Data)
    buffer = device->newBuffer(desc.Data, desc.Size, MTL::ResourceStorageModeShared);
  else
    buffer = device->newBuffer(size, MTL::ResourceStorageModeShared);

  buffer->setLabel(NS::String::alloc()->init(desc.DebugName.c_str(), NS::UTF8StringEncoding));
}

MetalBuffer::~MetalBuffer()
{
  buffer->release();
}

void MetalBuffer::SetData(std::size_t s, void* data, std::size_t offset)
{
  SDL_assert(offset + s <= size);
  char* bufferAddr = static_cast<char*>(buffer->contents());
  std::memcpy(bufferAddr + offset, data, s);
}

void MetalBuffer::Reset(MTL::Device* device, std::size_t s)
{
  if (s <= size)
    return; // no shrinking

  size = s;
  buffer->release();
  buffer = device->newBuffer(size, MTL::ResourceStorageModeShared);
  buffer->setLabel(NS::String::alloc()->init(name.c_str(), NS::UTF8StringEncoding));
}

} // namespace Vision
