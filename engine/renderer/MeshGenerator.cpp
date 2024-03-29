#include "MeshGenerator.h"

namespace Vision::MeshGenerator
{

Mesh* CreatePlaneMesh(float width, float height, float rows, float columns)
{
  // calculate general info about the mesh
  std::size_t verticesX = rows + 1;
  std::size_t verticesY = columns + 1;
  std::size_t numVertices = verticesX * verticesY;
  std::size_t numIndices = 6 * rows * columns; // 6 indices per quad

  // generate the vertices
  std::vector<Vision::MeshVertex> vertices(numVertices);
  float x = -width / 2.0f;
  for (std::size_t i = 0; i < verticesX; i++)
  {
    float y = -height / 2.0f;
    for (std::size_t j = 0; j < verticesY; j++)
    {
      Vision::MeshVertex vertex;
      vertex.Position = {x, y, 0.0f};
      vertex.Normal = {0.0f, 1.0f, 0.0f};
      vertex.Color = {0.2f, 0.2f, 0.6f, 1.0f};
      vertex.UV = {static_cast<float>(i) / static_cast<float>(verticesX), static_cast<float>(j) / static_cast<float>(verticesY)};
      vertices[i * verticesX + j] = vertex;

      y += height / static_cast<float>(columns);
    }
    x += width / static_cast<float>(rows);
  }

  // generate the indices
  std::vector<Vision::MeshIndex> indices(numIndices);
  std::size_t index = 0;
  for (std::size_t i = 0; i < rows; i++)
  {
    for (std::size_t j = 0; j < columns; j++)
    {
      Vision::MeshIndex current = static_cast<Vision::MeshIndex>(i * verticesX + j);
      Vision::MeshIndex right = static_cast<Vision::MeshIndex>(current + 1);
      Vision::MeshIndex above = static_cast<Vision::MeshIndex>(current + verticesX);
      Vision::MeshIndex diagonal = static_cast<Vision::MeshIndex>(right + verticesX);

      indices[index + 0] = current;
      indices[index + 1] = right;
      indices[index + 2] = diagonal;
      indices[index + 3] = current;
      indices[index + 4] = diagonal;
      indices[index + 5] = above;

      index += 6;
    }
  }

  // create the mesh descriptor
  Vision::MeshDesc desc;
  desc.Vertices = vertices;
  desc.NumVertices = numVertices;
  desc.Indices = indices;
  desc.NumIndices = numIndices;

  // return the new mesh
  return new Vision::Mesh(desc);
}

Mesh* CreateCubeMesh(float size)
{
  std::vector<glm::vec3> positions = {
    { -1.0f, -1.0f, -1.0f },
    { 1.0f, -1.0f, -1.0f },
    { 1.0f, 1.0f, -1.0f },
    { -1.0f, 1.0f, -1.0f },
    { -1.0f, -1.0f, 1.0f },
    { 1.0f, -1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f },
    { -1.0f, 1.0f, 1.0f }
  };

  std::vector<glm::vec2> uv = {
    {0.0f, 0.0f},
    {1.0f, 0.0f},
    {1.0f, 1.0f},
    {0.0f, 1.0f}
  };

  std::vector<glm::vec3> normals = 
  {
    { 0.0f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, -1.0f },
    { -1.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, -1.0f, 0.0f }
  };

  std::vector<MeshIndex> indices =
  {
    0, 1, 3, 3, 1, 2,
    1, 5, 2, 2, 5, 6,
    5, 4, 6, 6, 4, 7,
    4, 0, 7, 7, 0, 3,
    3, 2, 7, 7, 2, 6,
    4, 5, 0, 0, 5, 1
  };

  MeshIndex texInds[6] = {0, 1, 3, 3, 1, 2};

  MeshDesc desc;
  desc.NumVertices = indices.size();
  desc.NumIndices = indices.size();

  std::vector<MeshVertex> vertices(desc.NumVertices);
  for (std::size_t i = 0; i < indices.size(); ++i)
  {
    vertices[i].Position = positions[indices[i]] * size;
    vertices[i].Color = { 1.0f, 1.0f, 1.0f, 1.0f};
    vertices[i].Normal = normals[i / 6];
    vertices[i].UV = uv[texInds[i / 6]];
  }  
  desc.Vertices = vertices;

  for (std::size_t i = 0; i < indices.size(); ++i)
  {
    indices[i] = i;
  }
  desc.Indices = indices;

  return new Mesh(desc);
}

Mesh* CreateSphereMesh(float radius, std::size_t numSudivisions)
{
  // TODO: Spheres
  return nullptr;
}


}