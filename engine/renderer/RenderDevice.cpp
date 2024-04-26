#include "RenderDevice.h"

#include <SDL.h>

#include "opengl/GLDevice.h"

#include "ShaderCompiler.h"

namespace Vision
{

APIDevice* RenderDevice::device = nullptr;
RenderAPI RenderDevice::api = RenderAPI::None;

void RenderDevice::Init(RenderAPI renderAPI)
{
  api = renderAPI;
  SDL_assert(api == RenderAPI::OpenGL);

  SDL_assert(!device);
  device = new GLDevice();
}

void RenderDevice::Shutdown()
{
  delete device;
}

ID RenderDevice::CreatePipeline(const PipelineDesc& desc)
{
  return device->CreatePipeline(desc);
}

void RenderDevice::DestroyPipeline(ID pipeline)
{
  device->DestroyPipeline(pipeline);
}

ID RenderDevice::CreateShader(const ShaderDesc& desc)
{
  // we need to modify the desc, so we copy. it's fine b/c this is init code anyways.
  ShaderDesc tmp = desc; 

  ShaderCompiler compiler;
  compiler.GenerateStageMap(tmp);
  compiler.GenerateSPIRVMap(tmp);

  return device->CreateShader(tmp);
}

void RenderDevice::DestroyShader(ID shader)
{
  device->DestroyShader(shader);
}

ID RenderDevice::CreateBuffer(const BufferDesc &desc)
{
  return device->CreateBuffer(desc);
}

void RenderDevice::SetBufferData(ID buffer, void *data, std::size_t size)
{
  device->SetBufferData(buffer, data, size);
}

void RenderDevice::ResizeBuffer(ID buffer, std::size_t size)
{
  device->ResizeBuffer(buffer, size);
}

void RenderDevice::AttachUniformBuffer(ID buffer, std::size_t block)
{
  device->AttachUniformBuffer(buffer, block);
}

void RenderDevice::DestroyBuffer(ID id)
{
  device->DestroyBuffer(id);
}

ID RenderDevice::CreateTexture2D(const Texture2DDesc &desc)
{
  return device->CreateTexture2D(desc);
}

void RenderDevice::ResizeTexture2D(ID id, float width, float height)
{
  device->ResizeTexture2D(id, width, height);
}

void RenderDevice::SetTexture2DData(ID id, uint8_t *data)
{
  device->SetTexture2DData(id, data);
}

void RenderDevice::BindTexture2D(ID id, std::size_t binding)
{
  device->BindTexture2D(id, binding);
}

void RenderDevice::DestroyTexture2D(ID id)
{
  device->DestroyTexture2D(id);
}

ID RenderDevice::CreateCubemap(const CubemapDesc &desc)
{
  return device->CreateCubemap(desc);
}

void RenderDevice::BindCubemap(ID id, std::size_t binding)
{
  device->BindCubemap(id, binding);
}

void RenderDevice::DestroyCubemap(ID id)
{
  device->DestroyCubemap(id);
}

ID RenderDevice::CreateFramebuffer(const FramebufferDesc &desc)
{
  return device->CreateFramebuffer(desc);
}

void RenderDevice::ResizeFramebuffer(ID id, float width, float height)
{
  device->ResizeFramebuffer(id, width, height);
}

void RenderDevice::DestroyFramebuffer(ID id)
{
  device->DestroyFramebuffer(id);
}

ID RenderDevice::CreateRenderPass(const RenderPassDesc &desc)
{
  return device->CreateRenderPass(desc);
}

void RenderDevice::BeginRenderPass(ID pass)
{
  device->BeginRenderPass(pass);
}

void RenderDevice::EndRenderPass()
{
  device->EndRenderPass();
}

void RenderDevice::DestroyRenderPass(ID pass)
{
  device->DestroyRenderPass(pass);
}

void RenderDevice::Submit(const DrawCommand& command)
{
  device->Submit(command);
}

}