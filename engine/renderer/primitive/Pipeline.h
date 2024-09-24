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

enum class GeometryFillMode
{
  Line,
  Fill
};

struct RenderPipelineDesc
{
  // VBO Layouts
  std::vector<BufferLayout> Layouts;

  // Shaders
  ShaderSPIRV VertexShader;
  ShaderSPIRV PixelShader;

  // Geometry Fill Mode
  GeometryFillMode FillMode = GeometryFillMode::Fill;

  // Depth, Blending and PixelFormats
  PixelType PixelType;
  bool DepthTest = true;
  DepthFunc DepthFunc = DepthFunc::Less;
  bool DepthWrite = true;
  bool Blending = true; // TODO: Blend Modes

  // Tesselation
  bool UseTesselation = false;
  ShaderSPIRV HullShader;
  ShaderSPIRV DomainShader;

  RenderPipelineDesc() // default ctor to automatically set pixel format
      : PixelType(PixelType::BGRA8)
  {
  }
};

struct ComputePipelineDesc
{
  std::vector<ShaderSPIRV> ComputeKernels;
};

} // namespace Vision