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
  
  void Resize(float width, float height);

  void Begin(PerspectiveCamera* camera);
  void End();

  void DrawMesh(Mesh* mesh, Shader* shader, const glm::mat4& transform = glm::mat4(1.0f));

private:  
  bool m_InFrame = false;
  PerspectiveCamera* m_Camera = nullptr;
  float m_PixelDensity = 1.0f;
  float m_Width, m_Height;
};

}
