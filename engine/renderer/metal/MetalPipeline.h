#pragma once

#include <Metal/Metal.hpp>

#include "renderer/primitive/Pipeline.h"
#include "renderer/primitive/ObjectCache.h"

#include "MetalShader.h"

namespace Vision
{

class MetalPipeline
{
public:
  MetalPipeline(MTL::Device* device, ObjectCache<MetalShader>& shaders, const PipelineDesc& desc);
  ~MetalPipeline();

private:
  MTL::RenderPipelineState* pipeline; 
};

}