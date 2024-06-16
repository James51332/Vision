#pragma once

#include <Metal/Metal.hpp>
#include <QuartzCore/CAMetalLayer.hpp>

#include "renderer/RenderDevice.h"
#include "renderer/primitive/ObjectCache.h"

#include "MetalBuffer.h"
#include "MetalShader.h"
#include "MetalPipeline.h"
#include "MetalTexture.h"
#include "MetalRenderPass.h"
#include "MetalFramebuffer.h"

namespace Vision
{

class MetalDevice : public RenderDevice
{
public:
  MetalDevice(MTL::Device* device, CA::MetalLayer* layer);
  ~MetalDevice();

  ID CreatePipeline(const PipelineDesc& desc);
  void DestroyPipeline(ID id) { pipelines.Destroy(id); }

  ID CreateShader(const ShaderDesc& desc);
  void DestroyShader(ID id) { shaders.Destroy(id); }

  ID CreateBuffer(const BufferDesc &desc);
  void SetBufferData(ID buffer, void *data, std::size_t size) { buffers.Get(buffer)->SetData(size, data); }
  void MapBufferData(ID buffer, void** data, std::size_t size);
  void FreeBufferData(ID id, void **data);
  void ResizeBuffer(ID buffer, std::size_t size) { buffers.Get(buffer)->Reset(gpuDevice, size); }
  void AttachUniformBuffer(ID buffer, std::size_t block = 0);  
  void DestroyBuffer(ID id) { buffers.Destroy(id); }

  ID CreateTexture2D(const Texture2DDesc& desc);
  void ResizeTexture2D(ID id, float width, float height) { textures.Get(id)->Resize(gpuDevice, width, height); }
  void SetTexture2DData(ID id, uint8_t* data) { textures.Get(id)->SetData(data); }
  void BindTexture2D(ID id, std::size_t binding = 0);
  void DestroyTexture2D(ID id) { textures.Destroy(id); }

  ID CreateCubemap(const CubemapDesc& desc);
  void BindCubemap(ID id, std::size_t binding = 0);
  void DestroyCubemap(ID id) { cubemaps.Destroy(id); }

  ID CreateFramebuffer(const FramebufferDesc& desc);
  void ResizeFramebuffer(ID id, float width, float height);
  void DestroyFramebuffer(ID id) { framebuffers.Destroy(id); }

  ID CreateRenderPass(const RenderPassDesc& desc);
  void BeginRenderPass(ID pass);
  void EndRenderPass();
  void DestroyRenderPass(ID pass) { renderPasses.Destroy(pass); }

  void BeginCommandBuffer();
  void SubmitCommandBuffer(bool await = false);
  void SchedulePresentation();

  void SetViewport(float x, float y, float width, float height);
  void SetScissorRect(float x, float y, float width, float height);
  void Submit(const DrawCommand& command);

  // compute pipeline
  ID CreateComputePipeline(const ComputePipelineDesc& desc);
  void DestroyComputePipeline(ID id) { computePipelines.Destroy(id); }

  void BeginComputePass();
  void EndComputePass();

  void SetComputeBuffer(ID buffer, std::size_t binding = 0);
  void SetComputeTexture(ID texture, std::size_t binding = 0);

  void DispatchCompute(ID pipeline, const glm::vec3 &threads);

private:
  // gpu device
  MTL::Device* gpuDevice;

  // render target
  CA::MetalLayer* layer;
  CA::MetalDrawable* drawable = nullptr;

  // gpu data
  ID currentID = 1;
  ObjectCache<MetalBuffer> buffers;
  ObjectCache<MetalShader> shaders;
  ObjectCache<MetalPipeline> pipelines;
  ObjectCache<MetalTexture> textures;
  ObjectCache<MetalCubemap> cubemaps;
  ObjectCache<MetalRenderPass> renderPasses;
  ObjectCache<MetalFramebuffer> framebuffers;
  ObjectCache<MetalComputePipeline> computePipelines;

  // command stuff
  MTL::CommandQueue* queue = nullptr;
  MTL::CommandBuffer* cmdBuffer = nullptr;
  MTL::RenderCommandEncoder* encoder = nullptr;
  MTL::ComputeCommandEncoder* computeEncoder = nullptr;
};

}