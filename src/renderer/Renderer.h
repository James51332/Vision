#pragma once

#include <vector>
#include <glad/glad.h>

#include "Camera.h"
#include "Shader.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "Mesh.h"

namespace Vision
{

class Renderer
{
public:
  Renderer(float width, float height, float displayScale = 1.0f);
  ~Renderer();
  
  void Resize(float width, float height);

  void BeginFrame(OrthoCamera* camera, float boundSize);
  void EndFrame();

  void DrawPoint(const glm::vec2& position, const glm::vec4& color, float radius);

  void BeginScene(PerspectiveCamera* camera);
  void EndScene();

  void DrawMesh(Mesh* mesh, Shader* shader);

private:
  void Flush();
  
  void GenerateBuffers();
  void GenerateArrays();
  void GenerateShaders();

  void DestroyBuffers();
  void DestroyArrays();
  void DestroyShaders();

private:
  // General Rendering Data
  Buffer *m_QuadVBO, *m_QuadIBO;
  
  VertexArray *m_PointsVAO, *m_BackgroundVAO;
  Shader *m_PointShader, *m_BackgroundShader;
  
  // Point Instancing Data
  struct InstancedVertex
  {
    glm::vec2 Position;
    glm::vec4 Color;
    float Radius;
  };
  std::size_t m_Points = 0;
  std::size_t m_MaxPoints = 1000;

  Buffer* m_InstancedVBO;
  InstancedVertex* m_InstancedBuffer;
  
  // General Rendering Data
  bool m_InFrame = false, m_InScene = false;
  OrthoCamera* m_Camera = nullptr;
  PerspectiveCamera* m_SceneCamera = nullptr;
  float m_PixelDensity = 1.0f;
  float m_Width, m_Height;
};

}
