#pragma once 

#include "renderer/primitive/Pipeline.h"
#include "renderer/primitive/Buffer.h"
#include "renderer/primitive/Texture.h"
#include "renderer/primitive/RenderPass.h"

#include "renderer/shader/Shader.h"

#include "renderer/RenderAPI.h"
#include "renderer/RenderCommand.h"

namespace Vision
{

using ID = std::size_t;

class RenderDevice
{
public:
  static RenderDevice* Create(RenderAPI API);
  virtual ~RenderDevice() {}

  // render pipeline
  virtual ID CreateRenderPipeline(const RenderPipelineDesc& desc) = 0;
  virtual void DestroyPipeline(ID id) = 0;

  virtual ID CreateBuffer(const BufferDesc &desc) = 0;
  virtual void SetBufferData(ID buffer, void *data, std::size_t size) = 0;
  virtual void MapBufferData(ID buffer, void **data, std::size_t size) = 0;
  virtual void FreeBufferData(ID id, void **data) = 0;
  virtual void ResizeBuffer(ID buffer, std::size_t size) = 0;
  virtual void AttachUniformBuffer(ID buffer, std::size_t block = 0) = 0; 
  virtual void DestroyBuffer(ID id) = 0;

  virtual ID CreateTexture2D(const Texture2DDesc& desc) = 0;
  virtual void ResizeTexture2D(ID id, float width, float height) = 0;
  virtual void SetTexture2DData(ID id, uint8_t* data) = 0;
  virtual void BindTexture2D(ID id, std::size_t binding = 0) = 0;
  virtual void DestroyTexture2D(ID id) = 0;

  virtual ID CreateCubemap(const CubemapDesc& desc) = 0;
  virtual void BindCubemap(ID id, std::size_t binding = 0) = 0;
  virtual void DestroyCubemap(ID id) = 0;

  virtual ID CreateFramebuffer(const FramebufferDesc& desc) = 0;
  virtual void ResizeFramebuffer(ID id, float width, float height) = 0;
  virtual void DestroyFramebuffer(ID id) = 0;
  
  virtual ID CreateRenderPass(const RenderPassDesc& desc) = 0;
  virtual void BeginRenderPass(ID pass) = 0;
  virtual void EndRenderPass() = 0;
  virtual void DestroyRenderPass(ID pass) = 0; 

  virtual void BeginCommandBuffer() = 0;
  virtual void SubmitCommandBuffer(bool await = false) = 0;

  // TODO: This API could be much more explicit, but since this is
  // really all OpenGL has to offer, we'll use it. If we ever consider
  // supporting vulkan and are looking to squeeze the most performance
  // out of the renderer, we definitely can refine this system to saturate
  // the GPU much better.
  virtual void BufferBarrier() = 0;
  virtual void ImageBarrier() = 0;

  // used to present the next swapchain image to the screen.
  virtual void SchedulePresentation() = 0;
 
  virtual void SetViewport(float x, float y, float width, float height) = 0;
  virtual void SetScissorRect(float x, float y, float width, float height) = 0;
  virtual void Submit(const DrawCommand& command) = 0;

  // compute pipeline
  virtual ID CreateComputePipeline(const ComputePipelineDesc &desc) = 0;
  virtual void DestroyComputePipeline(ID id) = 0;

  virtual void BeginComputePass() = 0;
  virtual void EndComputePass() = 0;

  virtual void SetComputeBuffer(ID buffer, std::size_t binding = 0) = 0;
  virtual void SetComputeTexture(ID texture, std::size_t binding = 0) = 0;

  virtual void DispatchCompute(ID pipeline, const std::string& kernel, const glm::ivec3 &threadgroups) = 0;

  virtual RenderAPI GetRenderAPI() const = 0; 
};

}