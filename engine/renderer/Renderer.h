#pragma once

#include <vector>
#include <glad/glad.h>

#include "Camera.h"
#include "opengl/GLVertexArray.h"
#include "primitive/Shader.h"
#include "primitive/Buffer.h"
#include "Mesh.h"
#include "RenderCommand.h"
#include "RenderDevice.h"

namespace Vision
{

class Renderer
{
public:
  Renderer(float width, float height, float displayScale = 1.0f);
  
  void Resize(float width, float height);

  void Begin(Camera* camera);
  void End();

  void DrawMesh(Mesh* mesh, ID pipeline, const glm::mat4& transform = glm::mat4(1.0f));

  void Submit(const DrawCommand& command);

private:  
  bool m_InFrame = false;
  Camera* m_Camera = nullptr;
  float m_PixelDensity = 1.0f;
  float m_Width, m_Height;

  // Eventually, these will be baked into pipeline states, but for now we'll use a uniform buffer
  Buffer* pushConstants;
};

}
