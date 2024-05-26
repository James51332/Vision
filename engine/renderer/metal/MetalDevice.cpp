#include "MetalDevice.h"

#include <spirv_msl.hpp>
#include <iostream>
#include <SDL.h>

#include "renderer/ShaderCompiler.h"

#include "MetalType.h"

namespace Vision
{

MetalDevice::MetalDevice(MTL::Device *device)
  : gpuDevice(device)
{

}

MetalDevice::~MetalDevice()
{

}

ID MetalDevice::CreatePipeline(const PipelineDesc &desc)
{
  ID id = currentID++;
  MetalPipeline* ps = new MetalPipeline(gpuDevice, shaders, desc);
  pipelines.Add(id, ps);
  return id;
}

ID MetalDevice::CreateShader(const ShaderDesc &tmp)
{
  ID id = currentID++;
  MetalShader* shader;

  // we'll have all steps to build a shader in order and only enter the pipeline where
  // the user sets. the first stage is just to load the text and parse is into each
  // individual shader program.
  ShaderDesc desc = tmp;
  if (desc.Source == ShaderSource::File)
  {
    ShaderCompiler compiler;
    compiler.GenerateStageMap(desc);
    compiler.GenerateSPIRVMap(desc);
    desc.Source = ShaderSource::SPIRV;
  }

  // the second stage is to convert the raw shader source code into spirv, so we can perform
  // reflection and convert it to msl via spirv cross.
  if (desc.Source == ShaderSource::SPIRV)
  {
    // clear the stage map as we build the sources.
    desc.StageMap.clear();

    // iterate over each stage.
    for (auto pair : desc.SPIRVMap)
    {
      ShaderStage stage = pair.first;
      std::vector<uint32_t>& spirv = pair.second;

      // TODO: add options to this.
      spirv_cross::CompilerMSL compiler(spirv);
      std::string source = compiler.compile();
      
      desc.StageMap[stage] = source;

      // std::cout << ShaderStageToString(stage) << std::endl;
      // std::cout << source << std::endl << std::endl;
    }

    desc.Source = ShaderSource::StageMap;
  }

  // final stage is to to prepare the msl. note that this pipeline may potentially
  // want to incorporate the metal intermediate format for faster shader loading.
  // we could manufacture some sort of shader cache that is automatically built by
  // the engine. we'll need to make change for the shader descriptor to include the
  // stage map language since internal renderers provide shader source in GLSL.
  if (desc.Source == ShaderSource::StageMap)
  {
    shader = new MetalShader(gpuDevice, desc.StageMap); 
  }

  shaders.Add(id, shader);
  return id;
}

ID MetalDevice::CreateBuffer(const BufferDesc &desc)
{
  ID id = currentID++;
  MetalBuffer* buffer = new MetalBuffer(gpuDevice, desc);
  buffers.Add(id, buffer);
  return id;
}

void MetalDevice::AttachUniformBuffer(ID buffer, std::size_t block) 
{
  // TODO: The way we should implement this depends on how we transpile our shader
  // code. Thankfully, the user will never have to think about this, which is honestly
  // dope.
}

ID MetalDevice::CreateTexture2D(const Texture2DDesc &desc)
{
  ID id = currentID++;
  MetalTexture* texture;

  if (desc.LoadFromFile)
    texture = new MetalTexture(gpuDevice, desc.FilePath.c_str());
  else
    texture = new MetalTexture(gpuDevice, desc.Width, desc.Height, desc.PixelType);

  textures.Add(id, texture);
  return id;
}

void MetalDevice::BindTexture2D(ID id, std::size_t binding)
{

}

ID MetalDevice::CreateCubemap(const CubemapDesc &desc)
{
  ID id = currentID++;
  MetalCubemap* cubemap = new MetalCubemap(gpuDevice, desc);
  cubemaps.Add(id, cubemap);
  return id;
}

void MetalDevice::BindCubemap(ID id, std::size_t binding)
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