#include "MetalPipeline.h"

#include <iostream>
#include <SDL.h>
#include <spirv_msl.hpp>

#include "MetalCompiler.h"
#include "renderer/shader/ShaderReflector.h"

#include "MetalType.h"

namespace Vision
{

// ----- MetalPipeline -----

MetalPipeline::MetalPipeline(MTL::Device* device, const RenderPipelineDesc& desc)
{
  MTL::RenderPipelineDescriptor *attribs = MTL::RenderPipelineDescriptor::alloc()->init();

  // set the pixel format
  attribs->colorAttachments()->object(0)->setPixelFormat(PixelTypeToMTLPixelFormat(desc.PixelType));
  attribs->colorAttachments()->object(0)->setBlendingEnabled(desc.Blending);
  attribs->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
  attribs->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
  attribs->colorAttachments()->object(0)->setAlphaBlendOperation(MTL::BlendOperationAdd);
  attribs->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
  attribs->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
  attribs->colorAttachments()->object(0)->setRgbBlendOperation(MTL::BlendOperationAdd);

  // compile and attach our shader functions
  MetalCompiler shaderCompiler;

  MTL::Function* vertexFunc = shaderCompiler.Compile(device, desc.VertexShader);
  attribs->setVertexFunction(vertexFunc);
  vertexFunc->release();

  MTL::Function* fragmentFunc = shaderCompiler.Compile(device, desc.PixelShader);
  attribs->setFragmentFunction(fragmentFunc);
  fragmentFunc->release();

  // set the pipeline layout
  MTL::VertexDescriptor* vtxDesc = MTL::VertexDescriptor::alloc()->init();
  
  // build the free buffer bindings
  ShaderReflector vtxReflector(desc.VertexShader);
  std::vector<ShaderReflector::UniformBuffer> ubos = vtxReflector.GetUniformBuffers();
  constexpr std::size_t maxSlot = 30; // this is the last slot in the table
  stageBufferBindings.clear();
  
  for (std::size_t i = 0; i <= maxSlot; i++)
  {
    bool found = false;
    for (auto ubo : ubos) // nested loop isn't great
      found |= (ubo.Binding == i);

    if (found)
      continue;

    stageBufferBindings.push_back(i);

    if (stageBufferBindings.size() == desc.Layouts.size()) // we have enough slots.
      break;
  }

  int stageBuffer = 0;
  int attrib = 0;
  for (auto layout : desc.Layouts)
  {
    SDL_assert(stageBuffer < stageBufferBindings.size()); // We can't have more stage slots than free slots
    std::size_t layoutIndex = stageBufferBindings[stageBuffer];

    if (layout.Stride == 0)
    {
      stageBuffer++;
      continue;
    }
    
    for (auto elem : layout.Elements)
    {
      vtxDesc->attributes()->object(attrib)->setBufferIndex(layoutIndex);
      vtxDesc->attributes()->object(attrib)->setFormat(ShaderDataTypeToMTLVertexFormat(elem.Type, elem.Normalized));
      vtxDesc->attributes()->object(attrib)->setOffset(elem.Offset);

      attrib++;
    }

    vtxDesc->layouts()->object(layoutIndex)->setStride(layout.Stride);
    stageBuffer++;
  }

  attribs->setVertexDescriptor(vtxDesc);
  
  attribs->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float);

  // build the pipeline
  NS::Error *error = nullptr;
  pipeline = device->newRenderPipelineState(attribs, &error);

  if (error)
  {
    std::cout << "Failed to build render pipeline state!" << std::endl;
    std::cout << error->description()->cString(NS::UTF8StringEncoding) << std::endl;
  }

  vtxDesc->release();
  attribs->release();

  // create the depth state
  MTL::DepthStencilDescriptor* depthDesc = MTL::DepthStencilDescriptor::alloc()->init();  
  depthDesc->setDepthCompareFunction(desc.DepthTest ? DepthFunctionToMTLCompareFunction(desc.DepthFunc) : MTL::CompareFunctionAlways);
  depthDesc->setDepthWriteEnabled(desc.DepthWrite);

  depthState = device->newDepthStencilState(depthDesc);
  depthDesc->release();
}

MetalPipeline::~MetalPipeline()
{
  pipeline->release();
  depthState->release();
}

// ----- MetalComputePipeline -----

MetalComputePipeline::MetalComputePipeline(MTL::Device* device, const ComputePipelineDesc& desc)
{
  MetalCompiler compiler;
  NS::Error* error;

  for (auto& computeKernel : desc.ComputeKernels)
  {
    MTL::Function* kernelFunc = compiler.Compile(device, computeKernel);
    ShaderReflector reflector(computeKernel);

    Kernel kernel;
    kernel.Pipeline = device->newComputePipelineState(kernelFunc, &error);

    if (error)
    {
      std::cout << "Failed to generate compute pipeline state for kernel: " << computeKernel.Name;
      std::cout << error->description()->cString(NS::UTF8StringEncoding) << std::endl;
    }

    glm::ivec3 size = reflector.GetThreadgroupSize();
    kernel.WorkgroupSize = { NS::UInteger(size.x), NS::UInteger(size.y), NS::UInteger(size.z) };

    kernels[computeKernel.Name] = kernel;
  }
}

MetalComputePipeline::~MetalComputePipeline()
{
  for (auto& pair : kernels)
    pair.second.Pipeline->release();
}

MetalComputePipeline::Kernel MetalComputePipeline::GetKernel(const std::string &name)
{
  if (kernels.find(name) == kernels.end())
  {
    std::cout << "Unknown Compute Kernel: " << std::endl;
    
    Kernel k;
    k.Pipeline = nullptr;
    k.WorkgroupSize = { 1, 1, 1 };
    return k;
  }

  return kernels[name];
}

}
