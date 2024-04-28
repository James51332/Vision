#include "Mesh.h"

#include "core/App.h"

#include "RenderDevice.h"

namespace Vision
{

Mesh::Mesh(const MeshDesc& desc)
  : m_NumVertices(desc.NumVertices), m_NumIndices(desc.NumIndices)
{
  // Create vertex buffer
  {
    BufferDesc bufferDesc;
    bufferDesc.Type = BufferType::Vertex;
    bufferDesc.Data = (void*)desc.Vertices.data();
    bufferDesc.Size = desc.NumVertices * sizeof(MeshVertex);
    bufferDesc.Usage = BufferUsage::Dynamic;

    m_VertexBuffer = App::GetDevice()->CreateBuffer(bufferDesc);
  }

  // Create index buffer
  {
    BufferDesc bufferDesc;
    bufferDesc.Type = BufferType::Index;
    bufferDesc.Data = (void*)desc.Indices.data();
    bufferDesc.Size = desc.NumIndices * sizeof(MeshIndex);
    bufferDesc.Usage = BufferUsage::Dynamic;

    m_IndexBuffer = App::GetDevice()->CreateBuffer(bufferDesc);
  }
}

Mesh::~Mesh()
{
  App::GetDevice()->DestroyBuffer(m_VertexBuffer);
  App::GetDevice()->DestroyBuffer(m_IndexBuffer);
}

}