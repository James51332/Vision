#pragma once

#include <SDL.h>

#include "renderer/RenderDevice.h"
#include "renderer/primitive/ObjectCache.h"

#include "GLPipeline.h"
#include "GLProgram.h"
#include "GLVertexArray.h"
#include "GLBuffer.h"
#include "GLTexture.h"
#include "GLFramebuffer.h"

namespace Vision
{

class GLDevice : public RenderDevice
{
public:
  GLDevice(SDL_Window* wind);

  ID CreatePipeline(const PipelineDesc& desc);
  GLPipeline* GetPipeline(ID pipeline) { return pipelines.Get(pipeline); }
  void DestroyPipeline(ID pipeline) { pipelines.Destroy(pipeline); }

  ID CreateShader(const ShaderDesc &desc);
  GLProgram* GetShader(ID shader) { return shaders.Get(shader); }
  void DestroyShader(ID shader) { shaders.Destroy(shader); }

  ID CreateBuffer(const BufferDesc& desc);
  void SetBufferData(ID buffer, void* data, std::size_t size) { buffers.Get(buffer)->SetData(data, size); }
  void MapBufferData(ID buffer, void **data, std::size_t size);
  void FreeBufferData(ID id, void** data);
  void ResizeBuffer(ID buffer, std::size_t size) { buffers.Get(buffer)->Resize(size); }
  void AttachUniformBuffer(ID buffer, std::size_t block = 0) { buffers.Get(buffer)->Attach(block); }
  GLBuffer* GetBuffer(ID buffer) { return buffers.Get(buffer); }
  void DestroyBuffer(ID id) { buffers.Destroy(id); }

  ID CreateTexture2D(const Texture2DDesc &desc);
  void ResizeTexture2D(ID id, float width, float height) { textures.Get(id)->Resize(width, height); } 
  void SetTexture2DData(ID id, uint8_t *data) { textures.Get(id)->SetData(data); }
  void BindTexture2D(ID id, std::size_t binding = 0) { textures.Get(id)->Bind(binding); }
  GLTexture2D* GetTexture2D(ID id) { return textures.Get(id); }
  void DestroyTexture2D(ID id) { textures.Destroy(id); }

  ID CreateCubemap(const CubemapDesc& desc);
  void BindCubemap(ID id, std::size_t binding = 0) { cubemaps.Get(id)->Bind(binding); }
  void DestroyCubemap(ID id) { cubemaps.Destroy(id); }

  ID CreateFramebuffer(const FramebufferDesc &desc);
  void ResizeFramebuffer(ID id, float width, float height) { framebuffers.Get(id)->Resize(width, height); }
  GLFramebuffer* GetFramebuffer(ID id) { return framebuffers.Get(id); }
  void DestroyFramebuffer(ID id) { framebuffers.Destroy(id); }

  ID CreateRenderPass(const RenderPassDesc &desc);
  void BeginRenderPass(ID pass);
  void EndRenderPass();
  RenderPassDesc* GetRenderPass(ID id) { return renderpasses.Get(id); }
  void DestroyRenderPass(ID id) { renderpasses.Destroy(id); }

  virtual void SetViewport(float x, float y, float width, float height) { glViewport(x, y, width, height); }
  virtual void SetScissorRect(float x, float y, float width, float height);
  void Submit(const DrawCommand &command);

  void BeginCommandBuffer();
  void SubmitCommandBuffer(bool await = false);
  void SchedulePresentation();

  // compute pipeline
  ID CreateComputePipeline(const ComputePipelineDesc &desc) { return 0; }
  void DestroyComputePipeline(ID id) {}
  void BeginComputePass() {}
  void EndComputePass() {}
  void SetComputeBuffer(ID buffer, std::size_t binding = 0) {}
  void SetComputeTexture(ID texture, std::size_t binding = 0) {}
  void DispatchCompute(ID pipeline, const glm::vec3 &threads) {}

  RenderAPI GetRenderAPI() const { return RenderAPI::OpenGL; }

private:
  // swapchain image
  SDL_Window* window;

  // GPU data
  std::size_t currentID = 1;
  ObjectCache<GLPipeline> pipelines;
  ObjectCache<GLProgram> shaders;
  ObjectCache<GLBuffer> buffers;
  ObjectCache<GLTexture2D> textures;
  ObjectCache<GLCubemap> cubemaps;
  ObjectCache<GLFramebuffer> framebuffers;
  ObjectCache<RenderPassDesc> renderpasses;

  // vertex arrays aren't really real outside of opengl, so the engine caches them.
  // we hash to select one without having to rebuild each render.
  GLVertexArrayCache vaoCache;

  // renderer data
  ID activePass = 0;
  bool commandBufferActive = false;
  bool schedulePresent = false;
};

}