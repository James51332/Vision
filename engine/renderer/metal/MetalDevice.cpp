#include "MetalDevice.h"

#include <spirv_msl.hpp>
#include <iostream>
#include <SDL.h>

#include "renderer/ShaderCompiler.h"

#include "MetalType.h"

namespace Vision
{

MetalDevice::MetalDevice(MTL::Device *device, CA::MetalLayer* l, float w, float h)
  : gpuDevice(device->retain()), layer(l->retain()), width(w), height(h)
{
  queue = gpuDevice->newCommandQueue();
  cmdBuffer = nullptr;
  encoder = nullptr;

  depthTexture = new MetalTexture(gpuDevice, width, height, PixelType::Depth32);
}

MetalDevice::~MetalDevice()
{
  delete depthTexture;
  
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
  }

  if (desc.Source == ShaderSource::GLSL)
  {
    ShaderCompiler compiler;
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
      mslOpts.set_msl_version(2,0);
      compiler.set_msl_options(mslOpts);

      // uniform buffers take up the same space as stage buffers,
      // so we'll use the last buffer slots for our stage input.
      if (stage == ShaderStage::Vertex)
      {
        auto res = compiler.get_shader_resources();
        
        for (auto& buffer : res.uniform_buffers)
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
      // std::cout << ShaderStageToString(stage) << std::endl;
      // std::cout << source << std::endl << std::endl;
    }

    desc.Source = ShaderSource::MSL;
  }

  // final stage is to to prepare the msl. note that this pipeline may potentially
  // want to incorporate the metal intermediate format for faster shader loading.
  // we could manufacture some sort of shader cache that is automatically built by
  // the engine. we'll need to make change for the shader descriptor to include the
  // stage map language since internal renderers provide shader source in GLSL.
  if (desc.Source == ShaderSource::MSL)
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

void MetalDevice::MapBufferData(ID id, void **data, std::size_t size)
{
  MetalBuffer* buffer = buffers.Get(id);
  (*data) = buffer->buffer->contents();
}

void MetalDevice::FreeBufferData(ID id, void** data)
{
  (*data) = nullptr;
}

void MetalDevice::AttachUniformBuffer(ID buffer, std::size_t block) 
{
  // We don't know which we're setting so we have to set both.
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
  {
    texture = new MetalTexture(gpuDevice, desc.Width, desc.Height, desc.PixelType);
    if (desc.Data)
    	texture->SetData(desc.Data);
  }

  textures.Add(id, texture);
  return id;
}

void MetalDevice::BindTexture2D(ID id, std::size_t binding)
{
  // TODO: For now, we have no way to know which state, so we must do both.
  MetalTexture* texture = textures.Get(id);

  encoder->setVertexTexture(texture->GetTexture(), binding);
  encoder->setFragmentTexture(texture->GetTexture(), binding);

  encoder->setVertexSamplerState(texture->GetSampler(), binding);
  encoder->setFragmentSamplerState(texture->GetSampler(), binding);
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
  // TODO: For now, we have no way to know which state, so we must do both.
  MetalCubemap *texture = cubemaps.Get(id);

  encoder->setVertexTexture(texture->GetTexture(), binding);
  encoder->setFragmentTexture(texture->GetTexture(), binding);

  encoder->setVertexSamplerState(texture->GetSampler(), binding);
  encoder->setFragmentSamplerState(texture->GetSampler(), binding);
}

ID MetalDevice::CreateFramebuffer(const FramebufferDesc &desc)
{
  ID id = currentID++;
  MetalFramebuffer* fb = new MetalFramebuffer(gpuDevice, desc);
  framebuffers.Add(id, fb);
  return id;
}

void MetalDevice::ResizeFramebuffer(ID id, float width, float height)
{
  framebuffers.Get(id)->Resize(gpuDevice, width, height);
}

ID MetalDevice::CreateRenderPass(const RenderPassDesc &desc)
{
  ID id = currentID++;
  MetalRenderPass* rp = new MetalRenderPass(desc);
  renderPasses.Add(id, rp);
  return id;
}

void MetalDevice::BeginRenderPass(ID pass)
{
  SDL_assert(cmdBuffer);
  SDL_assert(!encoder);

  // Autoreleasepool ensures our command buffers don't hog memory when we're done with them.
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
  {
    // assign the proper texture to the descriptor.
    MetalRenderPass* renderpass = renderPasses.Get(pass);
    MTL::RenderPassDescriptor* rpDesc = renderpass->GetDescriptor();

    // if our target is the framebuffer, we need to fetch the drawable.
    if (renderpass->GetTarget() == 0)
    {
      if (!drawable) // only fetch if hasn't fetched since last presented.
      {
        drawable = layer->nextDrawable()->retain();
        drawablePresented = false;
      }

      rpDesc->colorAttachments()->object(0)->setTexture(drawable->texture());
    }
    else
    {
      MetalFramebuffer* fb = framebuffers.Get(renderpass->GetTarget());
      rpDesc->colorAttachments()->object(0)->setTexture(fb->GetTexture());
    }
    
    rpDesc->depthAttachment()->setTexture(depthTexture->GetTexture());
    rpDesc->depthAttachment()->setLoadAction(MTL::LoadActionClear);
    rpDesc->depthAttachment()->setStoreAction(MTL::StoreActionStore);

    encoder = cmdBuffer->renderCommandEncoder(rpDesc)->retain();
  }
  pool->release();
}

void MetalDevice::EndRenderPass()
{
  SDL_assert(cmdBuffer);
  SDL_assert(encoder);
  
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
  {
    encoder->endEncoding();
    encoder->release();
    encoder = nullptr;
  }
  pool->release();
}

void MetalDevice::BeginCommandBuffer()
{
  SDL_assert(!cmdBuffer);

  cmdBuffer = queue->commandBuffer();
}

void MetalDevice::SubmitCommandBuffer(bool await)
{
  SDL_assert(cmdBuffer);
  NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();
  {
    cmdBuffer->commit();
    if (await) cmdBuffer->waitUntilCompleted();
    cmdBuffer = nullptr;

    if (drawablePresented)
    {
      drawable->release();
      drawable = nullptr; // free the drawable only if presented.
    }
  }
  pool->release();
}

void MetalDevice::SchedulePresentation()
{
  SDL_assert(cmdBuffer);
  SDL_assert(drawable);

  cmdBuffer->presentDrawable(drawable);
  drawablePresented = true;
}

void MetalDevice::SetViewport(float x, float y, float width, float height)
{
  SDL_assert(encoder);
  MTL::Viewport viewport{ x, y, width, height, 0.0f, 1.0f };
  encoder->setViewport(viewport);
}

void MetalDevice::SetScissorRect(float x, float y, float width, float height)
{
  SDL_assert(encoder);
  MTL::ScissorRect rect {
    NS::UInteger(x), 
    NS::UInteger(y), 
    NS::UInteger(width), 
    NS::UInteger(height)
  };
  encoder->setScissorRect(rect);
}

void MetalDevice::Submit(const DrawCommand &command)
{
  SDL_assert(encoder);

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
  MTL::IndexType indexType = IndexTypeToMTLIndexType(command.IndexType);
  encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, command.NumVertices, indexType, indexBuffer->buffer, command.IndexOffset);
}

// compute API
ID MetalDevice::CreateComputePipeline(const ComputePipelineDesc &desc)
{
  ID id = currentID++;
  ComputePipelineDesc tmp = desc;
  MetalComputePipeline* pipeline = new MetalComputePipeline(gpuDevice, tmp);
  computePipelines.Add(id, pipeline);
  return id;
}

void MetalDevice::BeginComputePass()
{
  SDL_assert(cmdBuffer);
  SDL_assert(!encoder);
  SDL_assert(!computeEncoder);

  computeEncoder = cmdBuffer->computeCommandEncoder()->retain();
}

void MetalDevice::EndComputePass()
{
  SDL_assert(computeEncoder);

  NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();
  {
    computeEncoder->endEncoding();
    computeEncoder->release();
    computeEncoder = nullptr;
  }
  pool->release();
}

void MetalDevice::SetComputeBuffer(ID id, std::size_t binding)
{
  SDL_assert(computeEncoder);

  MetalBuffer* buffer = buffers.Get(id);
  computeEncoder->setBuffer(buffer->buffer, 0, binding);
}

void MetalDevice::SetComputeTexture(ID id, std::size_t binding)
{
  SDL_assert(computeEncoder);

  MetalTexture* texture = textures.Get(id);
  computeEncoder->setTexture(texture->GetTexture(), binding);
}

void MetalDevice::DispatchCompute(ID pipeline, const glm::vec3& threads)
{
  SDL_assert(computeEncoder);

  MetalComputePipeline* ps = computePipelines.Get(pipeline); 
  computeEncoder->setComputePipelineState(ps->GetPipeline());

  MTL::Size numThreads = { static_cast<NS::UInteger>(threads.x), static_cast<NS::UInteger>(threads.y), static_cast<NS::UInteger>(threads.z) };
  computeEncoder->dispatchThreads(numThreads, ps->GetWorkgroupSize());
}

void MetalDevice::UpdateSize(float w, float h)
{
  // Recreate the depth texture.
  width = w;
  height = h;
  depthTexture->Resize(gpuDevice, width, height);
}

}
