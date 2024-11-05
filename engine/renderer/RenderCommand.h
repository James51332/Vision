#pragma once

#include <glm/glm.hpp>

#include "primitive/Buffer.h"
#include "primitive/Pipeline.h"
#include "primitive/Texture.h"

#include "shader/Shader.h"

namespace Vision
{

enum class IndexType
{
  U8,
  U16,
  U32
};

enum class PrimitiveType
{
  Triangle,
  TriangleStrip,
  Patch
};

using ID = std::size_t;

struct DrawCommand
{
  PrimitiveType Type;

  ID RenderPipeline;

  std::vector<ID> VertexBuffers;
  ID IndexBuffer = 0;
  IndexType IndexType = IndexType::U32;

  std::size_t NumVertices = 0;
  std::vector<std::size_t> VertexOffsets;
  std::size_t IndexOffset = 0;
};

} // namespace Vision