#include "MetalDevice.h"

#include <SDL.h>
#include <dispatch/dispatch.h>
#include <iostream>
#include <spirv_msl.hpp>

#include "renderer/shader/ShaderCompiler.h"

#include "MetalType.h"

namespace Vision
{

// Dispatch Semaphore

struct DispatchSemaphore
{
  DispatchSemaphore(int value) { semaphore = dispatch_semaphore_create(value); }

  void Signal() { dispatch_semaphore_signal(semaphore); }
  void Wait() { dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER); }

private:
  dispatch_semaphore_t semaphore;
};

// Metal Device

MetalDevice::MetalDevice(MTL::Device* device, CA::MetalLayer* l, float w, float h)
    : gpuDevice(device->retain()), layer(l->retain()), width(w), height(h)
{
  queue = gpuDevice->newCommandQueue();
  cmdBuffer = nullptr;
  encoder = nullptr;

  depthTexture = new MetalTexture(gpuDevice, width, height, PixelType::Depth32Float,
                                  MinMagFilter::Linear, MinMagFilter::Linear,
                                  EdgeAddressMode::ClampToEdge, EdgeAddressMode::ClampToEdge);

  dispatchSemaphore = std::make_shared<DispatchSemaphore>(maxFramesInFlight);
}

MetalDevice::~MetalDevice()
{
  delete depthTexture;

  // These are all retain so they don't get delete before this class.
  layer->release();
  gpuDevice->release();
  queue->release();
}

ID MetalDevice::CreateRenderPipeline(const RenderPipelineDesc& desc)
{
  ID id = currentID++;
  MetalPipeline* ps = new MetalPipeline(gpuDevice, desc);
  pipelines.Add(id, ps);
  return id;
}

ID MetalDevice::CreateBuffer(const BufferDesc& desc)
{
  ID id = currentID++;
  MetalBuffer* buffer = new MetalBuffer(this, desc);
  buffers.Add(id, buffer);
  return id;
}

void MetalDevice::SetBufferData(ID buffer, void* data, std::size_t size, std::size_t offset)
{
  // This doesn't do anything if we are already in flight. Otherwise, it waits until the next buffer
  // to safely write to is done on the GPU.
  BeginFrameInFlight();

  buffers.Get(buffer)->SetData(this, size, data, offset);
}

void MetalDevice::MapBufferData(ID id, void** data, std::size_t size)
{
  MetalBuffer* buffer = buffers.Get(id);
  (*data) = buffer->GetActiveBuffer()->contents();
}

void MetalDevice::FreeBufferData(ID id, void** data)
{
  (*data) = nullptr;
}

void MetalDevice::BindBuffer(ID buffer, std::size_t block, std::size_t offset, std::size_t range)
{
  if (encoder)
  {
    encoder->setVertexBuffer(buffers.Get(buffer)->GetActiveBuffer(), offset, block);
    encoder->setFragmentBuffer(buffers.Get(buffer)->GetActiveBuffer(), offset, block);
  }
  else if (computeEncoder)
  {
    computeEncoder->setBuffer(buffers.Get(buffer)->GetActiveBuffer(), offset, block);
  }
}

ID MetalDevice::CreateTexture2D(const Texture2DDesc& desc)
{
  ID id = currentID++;
  MetalTexture* texture;

  if (desc.LoadFromFile)
    texture = new MetalTexture(gpuDevice, desc.FilePath.c_str(), desc.MinFilter, desc.MagFilter,
                               desc.AddressModeS, desc.AddressModeT);
  else
  {
    texture = new MetalTexture(gpuDevice, desc.Width, desc.Height, desc.PixelType, desc.MinFilter,
                               desc.MagFilter, desc.AddressModeS, desc.AddressModeT);
    if (desc.Data)
      texture->SetData(desc.Data);
  }

  textures.Add(id, texture);
  return id;
}

void MetalDevice::BindTexture2D(ID id, std::size_t binding)
{
  // TODO: For now, we have no way to know which state, so we must do both.
  MetalTexture* texture = textures.Get(id);

  encoder->setVertexTexture(texture->GetTexture(), binding);
  encoder->setFragmentTexture(texture->GetTexture(), binding);

  encoder->setVertexSamplerState(texture->GetSampler(), binding);
  encoder->setFragmentSamplerState(texture->GetSampler(), binding);
}

ID MetalDevice::CreateCubemap(const CubemapDesc& desc)
{
  ID id = currentID++;
  MetalCubemap* cubemap = new MetalCubemap(gpuDevice, desc);
  cubemaps.Add(id, cubemap);

  return id;
}

void MetalDevice::BindCubemap(ID id, std::size_t binding)
{
  // TODO: For now, we have no way to know which state, so we must do both.
  MetalCubemap* texture = cubemaps.Get(id);

  encoder->setVertexTexture(texture->GetTexture(), binding);
  encoder->setFragmentTexture(texture->GetTexture(), binding);

  encoder->setVertexSamplerState(texture->GetSampler(), binding);
  encoder->setFragmentSamplerState(texture->GetSampler(), binding);
}

ID MetalDevice::CreateFramebuffer(const FramebufferDesc& desc)
{
  // Creating the framebuffer object is easy.
  ID id = currentID++;
  MetalFramebuffer* fb = new MetalFramebuffer(gpuDevice, desc);
  framebuffers.Add(id, fb);

  // Now, we assign the textures to IDs.
  ID colorID = currentID++;
  textures.Add(colorID, fb->GetColorTexture());
  fb->SetColorID(colorID);

  ID depthID = currentID++;
  textures.Add(depthID, fb->GetDepthTexture());
  fb->SetDepthID(depthID);

  return id;
}

void MetalDevice::ResizeFramebuffer(ID id, float width, float height)
{
  // First, we delete the old textures.
  MetalFramebuffer* fb = framebuffers.Get(id);
  textures.Destroy(fb->GetColorID());
  textures.Destroy(fb->GetDepthID());

  // Now, we resize and reset the textures.
  fb->Resize(gpuDevice, width, height);
  textures.Add(fb->GetColorID(), fb->GetColorTexture());
  textures.Add(fb->GetDepthID(), fb->GetDepthTexture());
}

void MetalDevice::DestroyFramebuffer(ID id)
{
  // Delete the textures, since we are the owner of them.
  MetalFramebuffer* fb = framebuffers.Get(id);
  textures.Destroy(fb->GetColorID());
  textures.Destroy(fb->GetDepthID());
  framebuffers.Destroy(id);
}

ID MetalDevice::CreateRenderPass(const RenderPassDesc& desc)
{
  ID id = currentID++;
  MetalRenderPass* rp = new MetalRenderPass(desc);
  renderPasses.Add(id, rp);
  return id;
}

void MetalDevice::BeginRenderPass(ID pass)
{
  SDL_assert(cmdBuffer);
  SDL_assert(!encoder);

  // Autoreleasepool ensures our command buffers don't hog memory when we're done with them.
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
  {
    // assign the proper texture to the descriptor.
    MetalRenderPass* renderpass = renderPasses.Get(pass);
    MTL::RenderPassDescriptor* rpDesc = renderpass->GetDescriptor();

    // if our target is the framebuffer, we need to fetch the drawable.
    if (renderpass->GetTarget() == 0)
    {
      if (!drawable) // only fetch if hasn't fetched since last presented.
      {
        drawable = layer->nextDrawable()->retain();
        drawablePresented = false;

        // Also track our in flight frame.
        BeginFrameInFlight();
      }

      // Bind the window color buffer and the device's depth buffer.
      rpDesc->colorAttachments()->object(0)->setTexture(drawable->texture());
      rpDesc->depthAttachment()->setTexture(depthTexture->GetTexture());
    }
    else
    {
      // Bind the proper attachments.
      MetalFramebuffer* fb = framebuffers.Get(renderpass->GetTarget());
      rpDesc->colorAttachments()->object(0)->setTexture(fb->GetColorTexture()->GetTexture());
      rpDesc->depthAttachment()->setTexture(fb->GetDepthTexture()->GetTexture());
    }

    // As of now, we always clear the depth buffer.
    rpDesc->depthAttachment()->setLoadAction(MTL::LoadActionClear);
    rpDesc->depthAttachment()->setStoreAction(MTL::StoreActionStore);

    encoder = cmdBuffer->renderCommandEncoder(rpDesc)->retain();
  }
  pool->release();
}

void MetalDevice::EndRenderPass()
{
  SDL_assert(cmdBuffer);
  SDL_assert(encoder);

  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
  {
    encoder->endEncoding();
    encoder->release();
    encoder = nullptr;
  }
  pool->release();
}

void MetalDevice::BeginCommandBuffer()
{
  SDL_assert(!cmdBuffer);

  cmdBuffer = queue->commandBuffer();
}

void MetalDevice::SubmitCommandBuffer(bool await)
{
  SDL_assert(cmdBuffer);
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
  {
    // If we are presenting our drawable, we are going to schedule the end to the frame
    // in flight.
    if (drawablePresented)
      cmdBuffer->addCompletedHandler(
          [&](MTL::CommandBuffer* buffer)
          {
            if (dispatchSemaphore)
              dispatchSemaphore->Signal();
          });

    cmdBuffer->commit();
    if (await)
      cmdBuffer->waitUntilCompleted();
    cmdBuffer = nullptr;

    if (drawablePresented)
    {
      drawable->release();
      drawable = nullptr; // free the drawable only if presented.

      // We are no longer working on this render pass, so we end our in flight frame.
      inFlight = false;
    }
  }
  pool->release();
}

void MetalDevice::SchedulePresentation()
{
  SDL_assert(cmdBuffer);
  SDL_assert(drawable);

  cmdBuffer->presentDrawable(drawable);
  drawablePresented = true;
}

void MetalDevice::SetViewport(float x, float y, float width, float height)
{
  SDL_assert(encoder);
  MTL::Viewport viewport{x, y, width, height, 0.0f, 1.0f};
  encoder->setViewport(viewport);
}

void MetalDevice::SetScissorRect(float x, float y, float width, float height)
{
  SDL_assert(encoder);
  MTL::ScissorRect rect{NS::UInteger(x), NS::UInteger(y), NS::UInteger(width),
                        NS::UInteger(height)};
  encoder->setScissorRect(rect);
}

void MetalDevice::Submit(const DrawCommand& command)
{
  SDL_assert(encoder);

  // fetch the pipeline state
  MetalPipeline* ps = pipelines.Get(command.RenderPipeline);
  encoder->setRenderPipelineState(ps->GetPipeline());
  encoder->setTriangleFillMode(ps->GetFillMode());

  // setup our depth information
  encoder->setDepthStencilState(ps->GetDepthStencil());

  // bind our vertex buffers
  std::size_t numBuffers = command.VertexBuffers.size();
  auto& shaderStageBindings = ps->GetStageBufferBindings();

  SDL_assert(numBuffers <= shaderStageBindings.size());
  SDL_assert(command.VertexOffsets.empty() || command.VertexOffsets.size() == numBuffers);

  for (std::size_t i = 0; i < numBuffers; i++)
  {
    MetalBuffer* buffer = buffers.Get(command.VertexBuffers.at(i));
    std::size_t slot = shaderStageBindings.at(i);

    if (command.VertexOffsets.empty())
      encoder->setVertexBuffer(buffer->GetActiveBuffer(), 0, slot);
    else
      encoder->setVertexBuffer(buffer->GetActiveBuffer(), command.VertexOffsets[i], slot);
  }

  // submit the draw call.
  if (command.IndexBuffer)
  {
    MetalBuffer* indexBuffer = buffers.Get(command.IndexBuffer);
    MTL::IndexType indexType = IndexTypeToMTLIndexType(command.IndexType);
    encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, command.NumVertices, indexType,
                                   indexBuffer->GetActiveBuffer(), command.IndexOffset);
  }
  else
  {
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), command.NumVertices);
  }
}

// Compute API

ID MetalDevice::CreateComputePipeline(const ComputePipelineDesc& desc)
{
  ID id = currentID++;
  MetalComputePipeline* pipeline = new MetalComputePipeline(gpuDevice, desc);
  computePipelines.Add(id, pipeline);
  return id;
}

void MetalDevice::BeginComputePass()
{
  SDL_assert(cmdBuffer);
  SDL_assert(!encoder);
  SDL_assert(!computeEncoder);

  computeEncoder = cmdBuffer->computeCommandEncoder()->retain();
}

void MetalDevice::EndComputePass()
{
  SDL_assert(computeEncoder);

  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
  {
    computeEncoder->endEncoding();
    computeEncoder->release();
    computeEncoder = nullptr;
  }
  pool->release();
}

void MetalDevice::BindImage2D(ID id, std::size_t binding, ImageAccess access)
{
  if (encoder)
  {
    BindTexture2D(id, binding);
    return;
  }

  SDL_assert(computeEncoder || encoder);
  MetalTexture* texture = textures.Get(id);
  computeEncoder->setTexture(texture->GetTexture(), binding);
}

void MetalDevice::DispatchCompute(ID pipeline, const std::string& name, const glm::ivec3& groups)
{
  SDL_assert(computeEncoder);

  MetalComputePipeline* ps = computePipelines.Get(pipeline);
  MetalComputePipeline::Kernel kernel = ps->GetKernel(name);

  computeEncoder->setComputePipelineState(kernel.Pipeline);

  MTL::Size numGroups = {NS::UInteger(groups.x), NS::UInteger(groups.y), NS::UInteger(groups.z)};
  computeEncoder->dispatchThreadgroups(numGroups, kernel.WorkgroupSize);
}

void MetalDevice::UpdateSize(float w, float h)
{
  // Recreate the depth texture.
  width = w;
  height = h;
  depthTexture->Resize(gpuDevice, width, height);
}

void MetalDevice::BeginFrameInFlight()
{
  if (inFlight)
    return;

  // It may seem like a problem to wait on this semaphore, but if we are waiting on this semaphore,
  // our program seriously is GPU bound, so the performance impact should be negligible.
  dispatchSemaphore->Wait();
  inFlightFrame = (inFlightFrame + 1) % maxFramesInFlight;
  inFlight = true;
}

} // namespace Vision
