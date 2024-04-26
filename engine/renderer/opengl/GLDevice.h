#pragma once

#include "renderer/primitive/APIDevice.h"
#include "renderer/primitive/ObjectCache.h"

#include "GLPipeline.h"
#include "GLProgram.h"
#include "GLVertexArray.h"
#include "GLBuffer.h"
#include "GLTexture.h"
#include "GLFramebuffer.h"

namespace Vision
{

class GLDevice : public APIDevice
{
public:
  ID CreatePipeline(const PipelineDesc& desc);
  GLPipeline* GetPipeline(ID pipeline) { return pipelines.Get(pipeline); }
  void DestroyPipeline(ID pipeline) { pipelines.Destroy(pipeline); }

  ID CreateShader(const ShaderDesc &desc);
  GLProgram* GetShader(ID shader) { return shaders.Get(shader); }
  void DestroyShader(ID shader) { shaders.Destroy(shader); }

  ID CreateBuffer(const BufferDesc& desc);
  void SetBufferData(ID buffer, void* data, std::size_t size) { buffers.Get(buffer)->SetData(data, size); }
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

  void Submit(const DrawCommand &command);

private:
  ObjectCache<GLPipeline> pipelines;
  ObjectCache<GLProgram> shaders;
  ObjectCache<GLBuffer> buffers;
  ObjectCache<GLTexture2D> textures;
  ObjectCache<GLCubemap> cubemaps;
  ObjectCache<GLFramebuffer> framebuffers;

  // renderpasses are just equivalent to their descriptors in OpenGL
  ObjectCache<RenderPassDesc> renderpasses;
  ID activePass = 0;

  GLVertexArrayCache vaoCache;

  std::size_t currentID = 1;
};

}