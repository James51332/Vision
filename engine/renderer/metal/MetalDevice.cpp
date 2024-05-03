#include "MetalDevice.h"

namespace Vision
{

MetalDevice::MetalDevice(MTL::Device *device)
{

}

MetalDevice::~MetalDevice()
{

}

ID MetalDevice::CreatePipeline(const PipelineDesc &desc)
{
  return 0;
}

void MetalDevice::DestroyPipeline(ID id)
{

}

ID MetalDevice::CreateShader(const ShaderDesc &desc)
{
  return 0;
}

void MetalDevice::DestroyShader(ID id)
{

}

ID MetalDevice::CreateBuffer(const BufferDesc &desc)
{
  ID id = currentID++;
  MetalBuffer* buffer = new MetalBuffer(gpuDevice, desc);
  buffers.Add(id, buffer);
  return id;
}

void MetalDevice::SetBufferData(ID buffer, void *data, std::size_t size)
{
  buffers.Get(buffer)->SetData(size, data);
}

void MetalDevice::ResizeBuffer(ID buffer, std::size_t size)
{
  buffers.Get(buffer)->Reset(gpuDevice, size);
}

void MetalDevice::AttachUniformBuffer(ID buffer, std::size_t block) 
{

}

void MetalDevice::DestroyBuffer(ID id)
{
  buffers.Destroy(buffer);
}

ID MetalDevice::CreateTexture2D(const Texture2DDesc &desc)
{
  return 0;
}

void MetalDevice::ResizeTexture2D(ID id, float width, float height)
{

}

void MetalDevice::SetTexture2DData(ID id, uint8_t *data)
{

}

void MetalDevice::BindTexture2D(ID id, std::size_t binding)
{

}

void MetalDevice::DestroyTexture2D(ID id)
{

}

ID MetalDevice::CreateCubemap(const CubemapDesc &desc)
{
  return 0;
}

void MetalDevice::BindCubemap(ID id, std::size_t binding)
{

}

void MetalDevice::DestroyCubemap(ID id)
{

}

ID MetalDevice::CreateFramebuffer(const FramebufferDesc &desc)
{
  return 0;
}

void MetalDevice::ResizeFramebuffer(ID id, float width, float height)
{

}

void MetalDevice::DestroyFramebuffer(ID id)
{

}

ID MetalDevice::CreateRenderPass(const RenderPassDesc &desc)
{
  return 0;
}

void MetalDevice::BeginRenderPass(ID pass)
{

}

void MetalDevice::EndRenderPass()
{

}

void MetalDevice::DestroyRenderPass(ID pass)
{

}

void MetalDevice::SetViewport(float x, float y, float width, float height)
{

}

void MetalDevice::SetScissorRect(float x, float y, float width, float height)
{

}

void MetalDevice::Submit(const DrawCommand &command)
{

}

}