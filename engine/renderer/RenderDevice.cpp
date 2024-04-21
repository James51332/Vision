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

void RenderDevice::Submit(const DrawCommand& command)
{
  device->Submit(command);
}

}