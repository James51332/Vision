#include "MetalPipeline.h"

#include <iostream>
#include <SDL.h>

#include "MetalType.h"

namespace Vision
{

MetalPipeline::MetalPipeline(MTL::Device* device, ObjectCache<MetalShader>& shaders, const PipelineDesc& desc)
{
  MTL::RenderPipelineDescriptor *attribs = MTL::RenderPipelineDescriptor::alloc()->init();

  // set the pixel format
  attribs->colorAttachments()->object(0)->setPixelFormat(PixelTypeToMTLPixelFormat(desc.PixelFormat));

  // link the shader
  MetalShader *shader = shaders.Get(desc.Shader);
  SDL_assert(shader);
  attribs->setVertexFunction(shader->GetFunction(ShaderStage::Vertex));
  attribs->setFragmentFunction(shader->GetFunction(ShaderStage::Pixel));

  // set the pipeline layout
  MTL::VertexDescriptor* vtxDesc = MTL::VertexDescriptor::alloc()->init();
  
  // build the free buffer bindings
  auto& usedSlots = shader->GetUniformBufferSlots();
  constexpr std::size_t maxSlot = 30; // this is the last slot in the table
  stageBufferBindings.clear();
  
  for (std::size_t i = 0; i <= maxSlot; i++)
  {
    std::size_t count = std::count(usedSlots.begin(), usedSlots.end(), i);
    if (count == 1) 
      continue;
    else if (count == 0) 
      stageBufferBindings.push_back(i);
    else
      SDL_assert(false);

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
      vtxDesc->attributes()->object(attrib)->setFormat(ShaderDataTypeToMTLVertexFormat(elem.Type));
      vtxDesc->attributes()->object(attrib)->setOffset(elem.Offset);

      attrib++;
    }

    vtxDesc->layouts()->object(layoutIndex)->setStride(layout.Stride);
    stageBuffer++;
  }

  attribs->setVertexDescriptor(vtxDesc);

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
}

MetalPipeline::~MetalPipeline()
{
  pipeline->release();
}

}