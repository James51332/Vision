#pragma once

#include <glm/glm.hpp>

#include "primitive/Buffer.h"
#include "primitive/Shader.h"
#include "primitive/Texture.h"
#include "primitive/Pipeline.h"

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
  // Data
  ID Pipeline;
  std::vector<Buffer*> VertexBuffers;
  Buffer* IndexBuffer = nullptr;

  std::vector<Texture2D*> Textures;
  glm::mat4 Transform;

  // Settings
  IndexType IndexType = IndexType::U32;
  PrimitiveType Type;
  std::size_t NumVertices = 0;

  // Tesselation
  std::size_t PatchSize = 4;
};

}