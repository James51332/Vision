#pragma once

#include <Metal/Metal.hpp>

#include "renderer/primitive/Framebuffer.h"

namespace Vision
{

class MetalFramebuffer
{
public:
  MetalFramebuffer(MTL::Device* device, const FramebufferDesc& desc);
  ~MetalFramebuffer();

  void Resize(MTL::Device* device, float width, float height);

  MTL::Texture* GetTexture();

private:
  MTL::Texture* texture = nullptr;
  PixelType pixelType;
};

}