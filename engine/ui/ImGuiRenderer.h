#pragma once

#include <imgui.h>

#include <glad/glad.h>

#include "renderer/primitive/Buffer.h"
#include "renderer/primitive/Shader.h"
#include "renderer/opengl/GLVertexArray.h"

namespace Vision
{

class ImGuiRenderer
{
public:
  ImGuiRenderer(float width, float height, float displayScale = 1.0f);
  ~ImGuiRenderer();

  void Begin();
  void End();

  void Resize(float width, float height);

private:
  void GenerateBuffers();
  void GenerateArrays();
  void GenerateShaders();
  void GenerateTextures();

  void DestroyBuffers();
  void DestroyArrays();
  void DestroyShaders();
  void DestroyTextures();

private:
  GLBuffer *m_VBO, *m_IBO;
  GLVertexArray *m_VertexArray;
  GLProgram *m_Shader;
  GLuint m_FontTexture;

  std::size_t m_MaxVertices = 1000000; // 5MB GPU buffer
  std::size_t m_MaxIndices = 1000000;

  float m_PixelDensity = 1.0f;
  float m_Width, m_Height;
};

}