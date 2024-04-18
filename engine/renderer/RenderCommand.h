#pragma once

#include <glm/glm.hpp>

#include "Buffer.h"
#include "Shader.h"
#include "Texture.h"
#include "opengl/VertexArray.h"

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
  GLVertexArray* GLVertexArray = nullptr;
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