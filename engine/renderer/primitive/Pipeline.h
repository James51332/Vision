#pragma once

#include <vector>

#include "renderer/primitive/BufferLayout.h"
#include "renderer/primitive/Texture.h"
#include "renderer/shader/Shader.h"

namespace Vision
{

using ID = std::size_t;

enum class DepthFunc
{
  Less,
  LessEqual,
  Greater,
  GreaterEqual,
  Equal
};

struct RenderPipelineDesc
{
  // VBO Layouts
  std::vector<BufferLayout> Layouts;

  // Shaders
  ShaderSPIRV VertexShader;
  ShaderSPIRV PixelShader;

  // Depth, Blending and PixelFormats
  PixelType PixelType;
  bool DepthTest = true;
  DepthFunc DepthFunc = DepthFunc::Less;
  bool DepthWrite = true;
  bool Blending = true; // TODO: Blend Modes

  RenderPipelineDesc() // default ctor to automatically set pixel format
    : PixelType(PixelType::BGRA8) {}
};

struct ComputePipelineDesc
{
  std::vector<ShaderSPIRV> ComputeKernels;
};

}