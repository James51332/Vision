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

  MTL::RenderPipelineState* GetPipeline() const { return pipeline; }
  std::vector<std::size_t>& GetStageBufferBindings() { return stageBufferBindings; }
  MTL::DepthStencilState* GetDepthStencil() const { return depthState; }

private:
  MTL::RenderPipelineState* pipeline;
  std::vector<std::size_t> stageBufferBindings;
  MTL::DepthStencilState* depthState;
};

}