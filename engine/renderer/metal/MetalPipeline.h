#pragma once

#include <Metal/Metal.hpp>

#include "renderer/primitive/ObjectCache.h"
#include "renderer/primitive/Pipeline.h"

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
  MTL::TriangleFillMode GetFillMode() const { return fillMode; }

private:
  MTL::RenderPipelineState* pipeline;
  std::vector<std::size_t> stageBufferBindings;
  MTL::DepthStencilState* depthState;
  MTL::TriangleFillMode fillMode;
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
  MetalComputePipeline(MTL::Device* device, const ComputePipelineDesc& desc);
  ~MetalComputePipeline();

  Kernel GetKernel(const std::string& name);

private:
  std::unordered_map<std::string, Kernel> kernels;
};

} // namespace Vision