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
  
  int layoutIndex = 0;
  int attrib = 0;
  for (auto layout : desc.Layouts)
  {
    if (layout.Stride == 0)
    {
      layoutIndex++;
      continue;
    }
    
    for (auto elem : layout.Elements)
    {
      vtxDesc->attributes()->object(attrib)->setBufferIndex(layoutIndex);
      vtxDesc->attributes()->object(attrib)->setFormat(ShaderDataTypeToMTLVertexFormat(elem.Type));
      vtxDesc->attributes()->object(attrib)->setOffset(elem.Offset);

      attrib++;
    }

    vtxDesc->layouts()->object(0)->setStride(layout.Stride);
    layoutIndex++;
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