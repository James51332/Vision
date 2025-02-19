#pragma once

#include <SDL.h>

#include "renderer/RenderDevice.h"
#include "renderer/primitive/ObjectCache.h"

#include "GLBuffer.h"
#include "GLFramebuffer.h"
#include "GLPipeline.h"
#include "GLProgram.h"
#include "GLTexture.h"
#include "GLVertexArray.h"

namespace Vision
{

class GLDevice : public RenderDevice
{
public:
  GLDevice(SDL_Window* wind, float w, float h);

  ID CreateRenderPipeline(const RenderPipelineDesc& desc);
  GLPipeline* GetPipeline(ID pipeline) { return pipelines.Get(pipeline); }
  void DestroyPipeline(ID pipeline) { pipelines.Destroy(pipeline); }

  ID CreateBuffer(const BufferDesc& desc);
  void SetBufferData(ID buffer, void* data, std::size_t size, std::size_t offset)
  {
    buffers.Get(buffer)->SetData(data, size, offset);
  }
  void MapBufferData(ID buffer, void** data, std::size_t size);
  void FreeBufferData(ID id, void** data);
  void ResizeBuffer(ID buffer, std::size_t size) { buffers.Get(buffer)->Resize(size); }
  void BindBuffer(ID buffer, std::size_t block = 0, std::size_t offset = 0, std::size_t size = 0)
  {
    buffers.Get(buffer)->Attach(block, offset, size);
  }
  GLBuffer* GetBuffer(ID buffer) { return buffers.Get(buffer); }
  void DestroyBuffer(ID id) { buffers.Destroy(id); }

  ID CreateTexture2D(const Texture2DDesc& desc);
  void ResizeTexture2D(ID id, float width, float height)
  {
    textures.Get(id)->Resize(width, height);
  }
  void SetTexture2DData(ID id, uint8_t* data) { textures.Get(id)->SetData(data); }
  void SetTexture2DDataRaw(ID id, void* data) { textures.Get(id)->SetDataRaw(data); }
  void BindTexture2D(ID id, std::size_t binding = 0) { textures.Get(id)->Bind(binding); }
  GLTexture2D* GetTexture2D(ID id) { return textures.Get(id); }
  void DestroyTexture2D(ID id) { textures.Destroy(id); }

  ID CreateCubemap(const CubemapDesc& desc);
  void BindCubemap(ID id, std::size_t binding = 0) { cubemaps.Get(id)->Bind(binding); }
  void DestroyCubemap(ID id) { cubemaps.Destroy(id); }

  ID CreateFramebuffer(const FramebufferDesc& desc);
  ID GetFramebufferColorTex(ID id) { return framebuffers.Get(id)->GetColorID(); }
  ID GetFramebufferDepthTex(ID id) { return framebuffers.Get(id)->GetDepthID(); }
  void ResizeFramebuffer(ID id, float width, float height);
  GLFramebuffer* GetFramebuffer(ID id) { return framebuffers.Get(id); }
  void DestroyFramebuffer(ID id);

  ID CreateRenderPass(const RenderPassDesc& desc);
  void BeginRenderPass(ID pass);
  void EndRenderPass();
  RenderPassDesc* GetRenderPass(ID id) { return renderpasses.Get(id); }
  void DestroyRenderPass(ID id) { renderpasses.Destroy(id); }

  virtual void SetViewport(float x, float y, float width, float height)
  {
    glViewport(x, y, width, height);
  }
  virtual void SetScissorRect(float x, float y, float width, float height);
  void Submit(const DrawCommand& command);

  // Only should be used for RAW dependencies, since GL automatically handles others.
  void BufferBarrier();
  void ImageBarrier();

  void BeginCommandBuffer();
  void SubmitCommandBuffer(bool await = false);
  void SchedulePresentation();

  // compute pipeline
  ID CreateComputePipeline(const ComputePipelineDesc& desc);
  void DestroyComputePipeline(ID id) { computePrograms.Destroy(id); }

  void BeginComputePass();
  void EndComputePass();

  void BindImage2D(ID texture, std::size_t binding = 0,
                   ImageAccess access = ImageAccess::ReadWrite);

  void DispatchCompute(ID pipeline, const std::string& kernel, const glm::ivec3& threadgroups);

  RenderAPI GetRenderAPI() const { return RenderAPI::OpenGL; }

private:
  friend class GLContext;
  void UpdateSize(float w, float h)
  {
    width = w;
    height = h;
    glViewport(0, 0, width, height);
  }

private:
  // swapchain image
  SDL_Window* window;
  GLint versionMajor, versionMinor;
  float width, height;

  // GPU data
  std::size_t currentID = 1;
  ObjectCache<GLPipeline> pipelines;
  ObjectCache<GLBuffer> buffers;
  ObjectCache<GLTexture2D> textures;
  ObjectCache<GLCubemap> cubemaps;
  ObjectCache<GLFramebuffer> framebuffers;
  ObjectCache<RenderPassDesc> renderpasses;
  ObjectCache<GLComputeProgram> computePrograms;

  // vertex arrays aren't really real outside of opengl, so the engine caches them.
  // we hash to select one without having to rebuild each render.
  GLVertexArrayCache vaoCache;

  // renderer data
  ID activePass = 0;
  bool commandBufferActive = false;
  bool computePass = false;

  bool schedulePresent = false;
};

} // namespace Vision