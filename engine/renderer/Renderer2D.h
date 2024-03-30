#pragma once

#include "Camera.h"
#include "Buffer.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Texture.h"

namespace Vision
{

struct QuadVertex
{
  glm::vec2 Position;
  glm::vec4 Color;
  glm::vec2 UV;
  std::uint32_t TextureID = 0;
  float TilingFactor = 1.0f;
};

struct PointVertex
{
  glm::vec2 Position;
  glm::vec4 Color;
  glm::vec2 UV;
  float Border = 0.0f; // If border is zero, point is filled in.
};

// Batched 2D Renderer (Note that this all renders to a single plane—overlapping shapes results in undefined behavior)
class Renderer2D
{
public:
  Renderer2D(float width, float height, float displayScale = 1.0f);
  ~Renderer2D();

  void Resize(float width, float height);

  // 2D Renderer API
  void Begin(Camera *camera, bool useTransform = false, const glm::mat4& globalTransform = glm::mat4(1.0f));
  void End();

  // Shapes
  void DrawPoint(const glm::vec2 &position, const glm::vec4 &color = glm::vec4(1.0f), float radius = 1.0f);
  void DrawSquare(const glm::vec2 &position, const glm::vec4& color = glm::vec4(1.0f), float size = 1.0f);
  void DrawQuad(const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f));
  void DrawTexturedQuad(const glm::mat4& transform, const glm::vec4& color, Texture2D* texture, float tilingFactor = 1.0f);
  
  // Outlines
  void DrawLine(const glm::vec2& pos1, const glm::vec2& pos2, const glm::vec4& color = glm::vec4(1.0f), float thickness = 1.0f);
  void DrawCircle(const glm::vec2& position, const glm::vec4& color = glm::vec4(1.0f), float radius = 1.0f, float thickness = 0.1f);
  void DrawBox(const glm::vec2& position, const glm::vec4& color = glm::vec4(1.0f), float size = 1.0f, float thickness = 0.1f);

private:
  void Flush();

  void GenerateBuffers();
  void GenerateArrays();
  void GenerateShaders();
  void GenerateTextures();

  void DestroyBuffers();
  void DestroyArrays();
  void DestroyShaders();
  void DestroyTextures();

private:
  // General Rendering Data
  bool m_InFrame = false;
  Camera *m_Camera = nullptr;
  float m_PixelDensity = 1.0f;
  float m_Width, m_Height;

  // Mode
  bool m_UseGlobalTransform = false;
  glm::mat4 m_GlobalTransform = glm::mat4(1.0f);

  // Quad Info
  const std::size_t m_MaxQuads = 10000;
  std::size_t m_NumQuads = 0;
  QuadVertex *m_QuadBuffer, *m_QuadBufferHead;
  Buffer *m_QuadVBO, *m_QuadIBO;
  VertexArray* m_QuadVAO;
  Shader* m_QuadShader;
  
  // Texture Info
  const std::size_t m_MaxTextures = 15;
  Texture2D* m_WhiteTexture;
  std::vector<Texture2D*> m_Textures;
  std::size_t m_NumUserTextures = 0;

  // Point Info
  const std::size_t m_MaxPoints = 10000;
  std::size_t m_NumPoints = 0;
  PointVertex *m_PointBuffer, *m_PointBufferHead;
  Buffer* m_PointVBO; // We can use the same ibo because the indices for points are the same
  VertexArray* m_PointVAO;
  Shader* m_PointShader;
};

}