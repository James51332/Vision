#pragma once

#include <Metal/Metal.hpp>

#include "renderer/RenderDevice.h"
#include "renderer/primitive/ObjectCache.h"

#include "MetalBuffer.h"
#include "MetalShader.h"
#include "MetalPipeline.h"
#include "MetalTexture.h"

namespace Vision
{

class MetalDevice : public RenderDevice
{
public:
  MetalDevice(MTL::Device* device);
  ~MetalDevice();

  ID CreatePipeline(const PipelineDesc& desc);
  void DestroyPipeline(ID id) { pipelines.Destroy(id); }

  ID CreateShader(const ShaderDesc& desc);
  void DestroyShader(ID id) { shaders.Destroy(id); }

  ID CreateBuffer(const BufferDesc &desc);
  void SetBufferData(ID buffer, void *data, std::size_t size) { buffers.Get(buffer)->SetData(size, data); }
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
  void DestroyCubemap(ID id);

  ID CreateFramebuffer(const FramebufferDesc& desc);
  void ResizeFramebuffer(ID id, float width, float height);
  void DestroyFramebuffer(ID id);

  ID CreateRenderPass(const RenderPassDesc& desc);
  void BeginRenderPass(ID pass);
  void EndRenderPass();
  void DestroyRenderPass(ID pass);

  void SetViewport(float x, float y, float width, float height);
  void SetScissorRect(float x, float y, float width, float height);
  void Submit(const DrawCommand& command);

private:
  MTL::Device* gpuDevice;

  ObjectCache<MetalBuffer> buffers;
  ObjectCache<MetalShader> shaders;
  ObjectCache<MetalPipeline> pipelines;
  ObjectCache<MetalTexture> textures;

  ID currentID = 1;
};

}