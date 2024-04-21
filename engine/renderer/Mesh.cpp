#include "Mesh.h"

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

    m_VertexBuffer = RenderDevice::CreateBuffer(bufferDesc);
  }

  // Create index buffer
  {
    BufferDesc bufferDesc;
    bufferDesc.Type = BufferType::Index;
    bufferDesc.Data = (void*)desc.Indices.data();
    bufferDesc.Size = desc.NumIndices * sizeof(MeshIndex);
    bufferDesc.Usage = BufferUsage::Dynamic;

    m_IndexBuffer = RenderDevice::CreateBuffer(bufferDesc);
  }
}

Mesh::~Mesh()
{
  RenderDevice::DestroyBuffer(m_VertexBuffer);
  RenderDevice::DestroyBuffer(m_IndexBuffer);
}

}