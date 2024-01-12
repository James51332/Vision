#include "Renderer.h"

#include <SDL.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include "core/Input.h"

namespace Vision
{

Renderer::Renderer(float width, float height, float displayScale)
  : m_Width(width), m_Height(height), m_PixelDensity(displayScale)
{
  // Resize the viewport (no need to use Resize() because we've already done everything else it does)
  glViewport(0, 0, static_cast<GLsizei>(width * displayScale), static_cast<GLsizei>(height * displayScale));
}

void Renderer::Resize(float width, float height)
{
  m_Width = width;
  m_Height = height;

  glViewport(0, 0, static_cast<GLsizei>(width * m_PixelDensity), static_cast<GLsizei>(height * m_PixelDensity));
}

void Renderer::Begin(PerspectiveCamera* camera)
{
  assert(!m_InFrame);

  m_InFrame = true;
  m_Camera = camera;

  // Enable Blending and Depth Testing
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);
}

void Renderer::End()
{
  assert(m_InFrame);

  m_InFrame = false;
  m_Camera = nullptr;
}

void Renderer::DrawMesh(Mesh* mesh, Shader* shader)
{
  assert(m_InFrame);

  shader->Use();
  shader->UploadUniformMat4(&m_Camera->GetViewProjectionMatrix()[0][0], "u_ViewProjection");
  shader->UploadUniformFloat3(&m_Camera->GetPosition()[0], "u_CameraPos");

  static float time = SDL_GetTicks() / 1000.0f;
  static float lastTime = SDL_GetTicks() / 1000.0f;
  float curTime = SDL_GetTicks() / 1000.0f;
  if (!Input::KeyDown(SDL_SCANCODE_Q))
    time += curTime - lastTime;
  lastTime = curTime;
  shader->UploadUniformFloat(time, "u_Time");
  
  mesh->Bind();
  glDrawElements(GL_TRIANGLES, mesh->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
}

}
