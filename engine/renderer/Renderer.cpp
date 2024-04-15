#include "Renderer.h"

#include <SDL.h>
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

void Renderer::Begin(Camera* camera)
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

void Renderer::DrawMesh(Mesh* mesh, Shader* shader, const glm::mat4& transform)
{
  assert(m_InFrame);

  RenderCommand command;
  command.VertexArray = mesh->m_VertexArray;
  command.IndexBuffer = mesh->m_IndexBuffer;
  command.NumVertices = mesh->GetNumIndices() == 0 ? mesh->GetNumVertices() : mesh->GetNumIndices();
  command.IndexType = IndexType::U32;
  command.Transform = transform;
  command.Shader = shader;
  command.UseTesselation = shader->UsesTesselation();

  if (command.UseTesselation)
  {
    command.Type = PrimitiveType::Patch;
    command.PatchSize = 4; // TODO: Other patch sizes
  }
  else
  {
    command.Type = PrimitiveType::Triangle;
  }

  Submit(command);
}

static GLenum IndexTypeToGLenum(IndexType type)
{
  switch (type)
  {
    case IndexType::U8: return GL_UNSIGNED_BYTE;
    case IndexType::U16: return GL_UNSIGNED_SHORT;
    case IndexType::U32: return GL_UNSIGNED_INT;
  }
}

static GLenum PrimitiveTypeToGLenum(PrimitiveType type)
{
  switch (type)
  {
    case PrimitiveType::Triangle: return GL_TRIANGLES;
    case PrimitiveType::TriangleStrip: return GL_TRIANGLE_STRIP;
    case PrimitiveType::Patch: return GL_PATCHES;
  }
}

void Renderer::Submit(const RenderCommand& command)
{
  assert(m_InFrame);
  
  // bind the shader and upload uniforms
  command.Shader->Use();
  {
    command.Shader->UploadUniformMat4(&m_Camera->GetViewProjectionMatrix()[0][0], "u_ViewProjection");
    command.Shader->UploadUniformMat4(&m_Camera->GetViewMatrix()[0][0], "u_View");
    command.Shader->UploadUniformMat4(&m_Camera->GetProjectionMatrix()[0][0], "u_Projection");
    command.Shader->UploadUniformMat4(&command.Transform[0][0], "u_Transform");
    command.Shader->UploadUniformFloat3(&m_Camera->GetPosition()[0], "u_CameraPos");
  
    glm::vec2 viewport = { m_Width, m_Height };
    command.Shader->UploadUniformFloat2(&viewport[0], "u_ViewportSize");
    
    static float time = SDL_GetTicks() / 1000.0f;
    static float lastTime = SDL_GetTicks() / 1000.0f;
    float curTime = SDL_GetTicks() / 1000.0f;
    if (!Input::KeyDown(SDL_SCANCODE_Q))
      time += curTime - lastTime;
    lastTime = curTime;
    command.Shader->UploadUniformFloat(time, "u_Time");
  }

  // bind the textures
  int index = 0;
  for (auto texture : command.Textures)
  {
    texture->Bind(index);
    index++;
  }

  // choose the primitive type and index type
  GLenum primitive = PrimitiveTypeToGLenum(command.Type);

  // bind the vertex array
  command.VertexArray->Bind();
  
  // set the patch size
  glPatchParameteri(GL_PATCH_VERTICES, command.PatchSize);

  // draw
  if (command.IndexBuffer)
  {
    GLenum indexType = IndexTypeToGLenum(command.IndexType);
    command.IndexBuffer->Bind();
    
    // TODO: Vtx Offsets
    glDrawElements(primitive, command.NumVertices, indexType, nullptr);
  }
  else
  {
    glDrawArrays(primitive, 0, command.NumVertices);
  }
}

}
