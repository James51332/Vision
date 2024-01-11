#pragma once

#include "Camera.h"

namespace Vision
{

class Renderer2D
{
public:
  Renderer2D(float width, float height, float displayScale = 1.0f);
  ~Renderer2D();

  void Resize(float width, float height);

  // 2D Renderer API
  void BeginFrame(OrthoCamera *camera);
  void EndFrame();

  void DrawPoint(const glm::vec2 &position, const glm::vec4 &color, float radius);

private:
  void Flush();

  void GenerateBuffers();
  void GenerateArrays();
  void GenerateShaders();

  void DestroyBuffers();
  void DestroyArrays();
  void DestroyShaders();

private:
  // 2D Points Rendering Data
  Buffer *m_QuadVBO, *m_QuadIBO;
  VertexArray *m_PointsVAO;
  Shader *m_PointShader;

  // Point Instancing Data
  struct InstancedVertex
  {
    glm::vec2 Position;
    glm::vec4 Color;
    float Radius;
  };
  std::size_t m_Points = 0;
  std::size_t m_MaxPoints = 1000;

  Buffer *m_InstancedVBO;
  InstancedVertex *m_InstancedBuffer;

  // General Rendering Data
  bool m_InFrame = false;
  OrthoCamera *m_Camera = nullptr;
  float m_PixelDensity = 1.0f;
  float m_Width, m_Height;
};

}