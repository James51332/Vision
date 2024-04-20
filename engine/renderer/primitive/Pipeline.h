#pragma once

#include <vector>

#include "renderer/primitive/BufferLayout.h"

namespace Vision
{

using ID = std::size_t;

enum class PipelineType
{
  Render
};

struct PipelineDesc
{
  PipelineType Type;
  ID Shader;
  std::vector<BufferLayout> Layouts;
};

}