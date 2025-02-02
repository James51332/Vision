#include "MetalFramebuffer.h"

#include "MetalType.h"

namespace Vision
{

MetalFramebuffer::MetalFramebuffer(MTL::Device* device, const FramebufferDesc& descriptor)
    : desc(descriptor)
{
  Resize(device, descriptor.Width, descriptor.Height);
}

void MetalFramebuffer::Resize(MTL::Device* device, float width, float height)
{
  // Resize the descriptor as well
  desc.Width = width;
  desc.Height = height;

  // We rely on the MetalDevice to delete these textures before we recreate.
  colorTexture = new MetalTexture(device, width, height, desc.ColorFormat, MinMagFilter::Linear,
                                  MinMagFilter::Linear, EdgeAddressMode::ClampToEdge,
                                  EdgeAddressMode::ClampToEdge);

  depthTexture = new MetalTexture(device, width, height, desc.DepthType, MinMagFilter::Linear,
                                  MinMagFilter::Linear, EdgeAddressMode::ClampToEdge,
                                  EdgeAddressMode::ClampToEdge);
}

} // namespace Vision