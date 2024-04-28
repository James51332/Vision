#include "Renderer.h"

#include <SDL.h>
#include <glm/gtc/matrix_transform.hpp>

#include "core/App.h"
#include "core/Input.h"

namespace Vision
{

struct PushConstant
{
  glm::mat4 view;
  glm::mat4 proj;
  glm::mat4 viewProj;
  glm::vec2 viewSize;
  float time;
};

Renderer::Renderer(float width, float height, float displayScale)
  : m_Width(width), m_Height(height), m_PixelDensity(displayScale)
{
  // Resize the viewport
  Resize(width, height);

  // TODO: Push constants will probably be parts of pipeline states.
  {
    BufferDesc desc;
    desc.Type = BufferType::Uniform;
    desc.Usage = BufferUsage::Dynamic;
    desc.Size = sizeof(PushConstant);
    desc.Data = nullptr;

    pushConstants = App::GetDevice()->CreateBuffer(desc);
  }
}

Renderer::~Renderer()
{
  App::GetDevice()->DestroyBuffer(pushConstants);
}

void Renderer::Resize(float width, float height)
{
  m_Width = width;
  m_Height = height;

  App::GetDevice()->SetViewport(0, 0, width * m_PixelDensity, height * m_PixelDensity);
}

void Renderer::Begin(Camera* camera)
{
  assert(!m_InFrame);

  m_InFrame = true;
  m_Camera = camera;
}

void Renderer::End()
{
  assert(m_InFrame);

  m_InFrame = false;
  m_Camera = nullptr;
}

void Renderer::DrawMesh(Mesh* mesh, ID pipeline, const glm::mat4& transform)
{
  assert(m_InFrame);

  DrawCommand command;
  command.Pipeline = pipeline;
  command.VertexBuffers = { mesh->m_VertexBuffer };
  command.IndexBuffer = mesh->m_IndexBuffer;
  command.NumVertices = mesh->GetNumIndices() == 0 ? mesh->GetNumVertices() : mesh->GetNumIndices();
  command.IndexType = IndexType::U32;
  command.Type = PrimitiveType::Triangle;
  command.Transform = transform;
  command.PatchSize = 4; // TODO: Other patch sizes

  Renderer::Submit(command);
}

void Renderer::Submit(const DrawCommand& command)
{
  assert(m_InFrame);

  static float time = SDL_GetTicks() / 1000.0f;
  static float lastTime = SDL_GetTicks() / 1000.0f;
  float curTime = SDL_GetTicks() / 1000.0f;
  if (!Input::KeyDown(SDL_SCANCODE_Q))
    time += curTime - lastTime;
  lastTime = curTime;

  // upload push constants
  // which we are gonna write as a uniform buffer for opengl.
  PushConstant data;
  data.view = m_Camera->GetViewMatrix();
  data.proj = m_Camera->GetProjectionMatrix();
  data.viewProj = m_Camera->GetViewProjectionMatrix();
  data.viewSize = { m_Width, m_Height};
  data.time = time;
  App::GetDevice()->SetBufferData(pushConstants, &data, sizeof(PushConstant));
  App::GetDevice()->AttachUniformBuffer(pushConstants, 0);

  // device submit
  App::GetDevice()->Submit(command);
}

}
