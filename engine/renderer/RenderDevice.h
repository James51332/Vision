#pragma once

#include "RenderAPI.h"

#include "primitive/APIDevice.h"
#include "primitive/Pipeline.h"
#include "primitive/Shader.h"
#include "primitive/Buffer.h"
#include "primitive/Texture.h"
#include "primitive/Framebuffer.h"

#include "RenderCommand.h"

namespace Vision
{

// I honestly think that OpenGL's ID system is not bad. We can then handle all
// of the memory management in the engine, and ensure that we don't leak anything.
// Therefore, we may consider implementing some form of templated cache class.
using ID = std::size_t; 

// Our render device will be a singleton, because we actually don't want to interface
// with more than one API. The only reason this may not be beneficial could be down
// the line when we start to work on multithreaded command encoding. However, a simple
// solution is the command buffer option, where we fetch them from the device singleton.
class RenderDevice
{
  friend class App;

  static void Init(RenderAPI api);
  static void Shutdown();
public:
  static ID CreatePipeline(const PipelineDesc& desc);
  static void DestroyPipeline(ID pipeline);

  static ID CreateShader(const ShaderDesc& desc);
  static void DestroyShader(ID shader);

  static ID CreateBuffer(const BufferDesc &desc);
  static void SetBufferData(ID buffer, void *data, std::size_t size);
  static void ResizeBuffer(ID buffer, std::size_t size);
  static void AttachUniformBuffer(ID buffer, std::size_t block = 0);
  static void DestroyBuffer(ID id);

  static ID CreateTexture2D(const Texture2DDesc &desc);
  static void ResizeTexture2D(ID id, float width, float height);
  static void SetTexture2DData(ID id, uint8_t *data);
  static void BindTexture2D(ID id, std::size_t binding = 0);
  static void DestroyTexture2D(ID id);

  static ID CreateCubemap(const CubemapDesc &desc);
  static void BindCubemap(ID id, std::size_t binding = 0);
  static void DestroyCubemap(ID id);

  // TODO: This should probably exist in some command encoder, but for now it's here.
  static void Submit(const DrawCommand& command);

private:
  static APIDevice* device; // This is a pointer to an implementation of this class for a certain API
  static RenderAPI api;
};

}