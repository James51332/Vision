#include "Mesh.h"

namespace Vision
{

Mesh::Mesh(const MeshDesc& desc)
  : m_NumVertices(desc.NumVertices), m_NumIndices(desc.NumIndices)
{
  // Create vertex buffer
  {
    BufferDesc bufferDesc;
    bufferDesc.Type = GL_ARRAY_BUFFER;
    bufferDesc.Data = (void*)desc.Vertices.data();
    bufferDesc.Size = desc.NumVertices * sizeof(MeshVertex);
    bufferDesc.Usage = GL_DYNAMIC_DRAW;
    bufferDesc.Layout = {
      { ShaderDataType::Float3, "Position"},
      { ShaderDataType::Float3, "Normal" },
      { ShaderDataType::Float4, "Color" },
      { ShaderDataType::Float2, "UV"}
    };

    m_VertexBuffer = new Buffer(bufferDesc);
  }

  // Create vertex array
  {
    m_VertexArray = new VertexArray();
    m_VertexArray->AttachBuffer(m_VertexBuffer);
  }

  // Create index buffer
  {
    BufferDesc bufferDesc;
    bufferDesc.Type = GL_ELEMENT_ARRAY_BUFFER;
    bufferDesc.Data = (void*)desc.Indices.data();
    bufferDesc.Size = desc.NumIndices * sizeof(MeshIndex);
    bufferDesc.Usage = GL_DYNAMIC_DRAW;

    m_IndexBuffer = new Buffer(bufferDesc);
  }
}

void Mesh::Bind()
{
  m_VertexArray->Bind();
  m_IndexBuffer->Bind();
}

}