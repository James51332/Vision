#pragma once

#include <Metal/Metal.hpp>

#include "renderer/primitive/Pipeline.h"
#include "renderer/primitive/ObjectCache.h"

namespace Vision
{

class MetalPipeline
{
public:
  MetalPipeline(MTL::Device* device, const RenderPipelineDesc& desc);
  ~MetalPipeline();

  MTL::RenderPipelineState* GetPipeline() const { return pipeline; }
  std::vector<std::size_t>& GetStageBufferBindings() { return stageBufferBindings; }
  MTL::DepthStencilState* GetDepthStencil() const { return depthState; }

private:
  MTL::RenderPipelineState* pipeline;
  std::vector<std::size_t> stageBufferBindings;
  MTL::DepthStencilState* depthState;
};

// compute pipeline
class MetalComputePipeline
{
public:
  struct Kernel
  {
    MTL::ComputePipelineState* Pipeline;
    MTL::Size WorkgroupSize;
  };

public:
  MetalComputePipeline(MTL::Device *device, const ComputePipelineDesc &desc);
  ~MetalComputePipeline();

  Kernel GetKernel(const std::string& name);

private:
  std::unordered_map<std::string, Kernel> kernels;
};

}