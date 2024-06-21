#pragma once

#include <Metal/Metal.hpp>

#include "renderer/primitive/RenderPass.h"

namespace Vision
{

class MetalRenderPass
{
public:
  MetalRenderPass(const RenderPassDesc& desc);
  ~MetalRenderPass();

  MTL::RenderPassDescriptor* GetDescriptor() const { return descriptor; }
  int GetTarget() const { return target; }

private:
  MTL::RenderPassDescriptor* descriptor;
  MTL::RenderPassDepthAttachmentDescriptor* depthAttachmentDesc;

  int target = 0;
};

}