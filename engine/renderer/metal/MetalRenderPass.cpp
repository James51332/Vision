#include "MetalRenderPass.h"

#include "MetalType.h"

namespace Vision
{

MetalRenderPass::MetalRenderPass(const RenderPassDesc& desc)
  : target(desc.Framebuffer)
{
  descriptor = MTL::RenderPassDescriptor::alloc()->init();

  MTL::ClearColor color = { desc.ClearColor.r, desc.ClearColor.g, desc.ClearColor.b, desc.ClearColor.a };
  descriptor->colorAttachments()->object(0)->setClearColor(color);
  descriptor->colorAttachments()->object(0)->setLoadAction(LoadOpToMTLLoadAction(desc.LoadOp));
  descriptor->colorAttachments()->object(0)->setStoreAction(StoreOpToMTLStoreAction(desc.StoreOp));

  depthAttachmentDesc = MTL::RenderPassDepthAttachmentDescriptor::alloc()->init();
  depthAttachmentDesc->setClearDepth(1.0f); // We have no API for this yet. Idk if we need it.
  descriptor->setDepthAttachment(depthAttachmentDesc);
}

MetalRenderPass::~MetalRenderPass()
{
  depthAttachmentDesc->release();
  descriptor->release();
}

}
