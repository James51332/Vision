#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "VertexArray.h"
#include "Buffer.h"

namespace Vision
{

struct MeshVertex
{
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec4 Color;
  glm::vec2 UV;
};

using MeshIndex = std::uint32_t;
  
struct MeshDesc
{
  std::size_t NumVertices;
  std::vector<MeshVertex> Vertices;

  std::size_t NumIndices;
  std::vector<MeshIndex> Indices;
};

class Mesh
{
public:
  Mesh(const MeshDesc& desc);

  void Bind();

  std::size_t GetNumIndices() const { return m_NumIndices; }
  std::size_t GetNumVertices() const { return m_NumVertices; }

private:
  VertexArray* m_VertexArray;
  Buffer* m_VertexBuffer;
  Buffer* m_IndexBuffer;

  std::size_t m_NumVertices;
  std::size_t m_NumIndices;
};

}