#include "MetalFramebuffer.h"

#include "MetalType.h"

namespace Vision
{

MetalFramebuffer::MetalFramebuffer(MTL::Device* device, const FramebufferDesc& desc)
  : pixelType(desc.ColorFormat)
{
  Resize(device, desc.Width, desc.Height);
}

MetalFramebuffer::~MetalFramebuffer()
{
  texture->release();
}

void MetalFramebuffer::Resize(MTL::Device* device, float width, float height)
{
  if (texture)
    texture->release();

  MTL::TextureDescriptor *descriptor;
  MTL::PixelFormat format = PixelTypeToMTLPixelFormat(pixelType);

  descriptor = MTL::TextureDescriptor::alloc()->texture2DDescriptor(format, width, height, false);
  descriptor->setUsage(MTL::TextureUsageRenderTarget);

  texture = device->newTexture(descriptor);

  descriptor->release();
}

MTL::Texture* MetalFramebuffer::GetTexture()
{
  return texture;
}

}