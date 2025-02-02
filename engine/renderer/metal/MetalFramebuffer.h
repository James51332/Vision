#pragma once

#include <Metal/Metal.hpp>

#include "MetalTexture.h"
#include "renderer/primitive/Framebuffer.h"

namespace Vision
{

class MetalFramebuffer
{
public:
  MetalFramebuffer(MTL::Device* device, const FramebufferDesc& desc);

  void Resize(MTL::Device* device, float width, float height);

  MetalTexture* GetColorTexture() const { return colorTexture; }
  MetalTexture* GetDepthTexture() const { return depthTexture; }
  const FramebufferDesc& GetDesc() const { return desc; }

  // Allow us to store the IDs for the color and depth textures within the framebuffer.
  ID GetColorID() const { return colorID; }
  ID GetDepthID() const { return depthID; }
  void SetColorID(ID id) { colorID = id; }
  void SetDepthID(ID id) { depthID = id; }

private:
  FramebufferDesc desc;

  // These textures are owned by the MetalDevice, but we can store their IDs here.
  ID colorID, depthID;
  MetalTexture* colorTexture = nullptr;
  MetalTexture* depthTexture = nullptr;
};

} // namespace Vision