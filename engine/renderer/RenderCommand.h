#pragma once

#include <glm/glm.hpp>

#include "Buffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

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

struct RenderCommand
{
  // Data
  VertexArray* VertexArray = nullptr;
  Buffer* IndexBuffer = nullptr;
  Shader* Shader;
  std::vector<Texture2D*> Textures;
  glm::mat4 Transform;

  // Settings
  IndexType IndexType = IndexType::U32;
  PrimitiveType Type;
  std::size_t NumVertices = 0;

  // Tesselation
  bool UseTesselation = false;
  std::size_t PatchSize;
};

}