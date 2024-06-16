#pragma once

#include <vector>

#include "renderer/primitive/BufferLayout.h"
#include "renderer/primitive/Texture.h"
#include "renderer/primitive/Shader.h"

namespace Vision
{

using ID = std::size_t;

enum class PipelineType
{
  Render
};

enum class DepthFunc
{
  Less,
  LessEqual,
  Greater,
  GreaterEqual,
  Equal
};

struct PipelineDesc
{
  PipelineType Type;
  ID Shader;
  std::vector<BufferLayout> Layouts;

  bool DepthTest = true;
  DepthFunc DepthFunc = DepthFunc::Less;
  bool DepthWrite = true;

  bool Blending = true;
  // TODO: Blend Modes

  PixelType PixelFormat;
  // TODO: Other pipeline features, and dynamic states?
};

struct ComputePipelineDesc
{
  ShaderSource Source = ShaderSource::File;

  std::string FilePath;
  std::string GLSL;
  std::vector<uint32_t> SPIRV;
};

}