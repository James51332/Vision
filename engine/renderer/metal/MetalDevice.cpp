#include "MetalDevice.h"

#include <spirv_msl.hpp>
#include <iostream>
#include <SDL.h>

#include "renderer/ShaderCompiler.h"

#include "MetalType.h"

namespace Vision
{

MetalDevice::MetalDevice(MTL::Device *device, CA::MetalLayer* l)
  : gpuDevice(device->retain()), layer(l->retain())
{
  queue = gpuDevice->newCommandQueue();
  cmdBuffer = nullptr;
  encoder = nullptr; 
}

MetalDevice::~MetalDevice()
{
  // These are all retain so they don't get delete before this class.
  layer->release();
  gpuDevice->release();
  queue->release();
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
  std::unordered_map<std::string, std::size_t> ubos;

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

      // create and configure the compiler
      spirv_cross::CompilerMSL compiler(std::move(spirv));

      auto mslOpts = compiler.get_msl_options();
      mslOpts.enable_decoration_binding = true; // set the compiler to use glsl bindings for buffer indices.
      compiler.set_msl_options(mslOpts);

      // uniform buffers take up the same space as stage buffers,
      // so we'll use the last buffer slots for our stage input.
      if (stage == ShaderStage::Vertex)
      {
        auto res = compiler.get_shader_resources();
        
        for (auto buffer : res.uniform_buffers)
        {
          auto slot = compiler.get_decoration(buffer.id, spv::DecorationBinding);
          std::string name = compiler.get_name(buffer.id);
          ubos[name] = slot;
        }
      }

      // generate the source code.
      std::string source = compiler.compile();
      desc.StageMap[stage] = source;

      // Log the generated shader code.
      std::cout << ShaderStageToString(stage) << std::endl;
      std::cout << source << std::endl << std::endl;
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
    shader = new MetalShader(gpuDevice, desc.StageMap, ubos); 
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
  encoder->setVertexBuffer(buffers.Get(buffer)->buffer, 0, block);
  encoder->setFragmentBuffer(buffers.Get(buffer)->buffer, 0, block);
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
  encoder->setFragmentSamplerState(temp, binding);
  encoder->setFragmentTexture(textures.Get(id)->GetTexture(), binding);
}

ID MetalDevice::CreateCubemap(const CubemapDesc &desc)
{
  ID id = currentID++;
  MetalCubemap* cubemap = new MetalCubemap(gpuDevice, desc);
  cubemaps.Add(id, cubemap);
  // Hack: sampler states should be part of textures
  MTL::SamplerDescriptor* descr = MTL::SamplerDescriptor::alloc()->init();
  descr->setMinFilter(MTL::SamplerMinMagFilterLinear);
  descr->setMagFilter(MTL::SamplerMinMagFilterLinear);
  temp = gpuDevice->newSamplerState(descr);
  descr->release();
  
  return id;
}

void MetalDevice::BindCubemap(ID id, std::size_t binding)
{
  encoder->setFragmentSamplerState(temp, binding);
  encoder->setFragmentTexture(cubemaps.Get(id)->GetTexture(), binding);
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

// For now the method is to just use a command encoder per renderpass.
void MetalDevice::BeginRenderPass(ID pass)
{
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
  
  cmdBuffer = queue->commandBuffer()->retain();

  MTL::RenderPassDescriptor* descriptor = MTL::RenderPassDescriptor::alloc()->init();
  descriptor->colorAttachments()->object(0)->setClearColor({0.0f, 0.0f, 0.0f, 1.0f});
  descriptor->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
  descriptor->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
  
  MTL::RenderPassDepthAttachmentDescriptor* da = MTL::RenderPassDepthAttachmentDescriptor::alloc()->init();
  da->setClearDepth(1.0f);
  descriptor->setDepthAttachment(da);
  da->release();

  drawable = layer->nextDrawable()->retain();
  descriptor->colorAttachments()->object(0)->setTexture(drawable->texture());

  encoder = cmdBuffer->renderCommandEncoder(descriptor)->retain();
  
  descriptor->release();
  
  pool->release();
}

void MetalDevice::EndRenderPass()
{
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
  
  assert(cmdBuffer);

  encoder->endEncoding();
  encoder->release();
  encoder = nullptr;

  cmdBuffer->presentDrawable(drawable);
  cmdBuffer->commit();
  cmdBuffer->release();
  cmdBuffer = nullptr;
  
  drawable->release();
  
  pool->release();
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
  assert(encoder != nullptr);

  // fetch the pipeline state
  MetalPipeline* ps = pipelines.Get(command.Pipeline);
  encoder->setRenderPipelineState(ps->GetPipeline());
  
  // setup our depth information
  encoder->setDepthStencilState(ps->GetDepthStencil());

  // bind our vertex buffers
  std::size_t numBuffers = command.VertexBuffers.size();
  auto& shaderStageBindings = ps->GetStageBufferBindings();

  SDL_assert(numBuffers <= shaderStageBindings.size());
  for (std::size_t i = 0; i < numBuffers; i++)
  {
    MetalBuffer* buffer = buffers.Get(command.VertexBuffers.at(i));
    std::size_t slot = shaderStageBindings.at(i);
    encoder->setVertexBuffer(buffer->buffer, 0, slot);
  }

  // submit the draw call.
  MetalBuffer* indexBuffer = buffers.Get(command.IndexBuffer);
  encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, command.NumVertices, MTL::IndexTypeUInt32, indexBuffer->buffer, 0);
}

}
