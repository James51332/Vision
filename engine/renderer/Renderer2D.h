#pragma once

#include "Camera.h"
#include "RenderDevice.h"

#include "primitive/Buffer.h"
#include "primitive/Texture.h"
#include "primitive/Pipeline.h"

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
  Renderer2D(RenderDevice* device, float width, float height, float displayScale = 1.0f);
  ~Renderer2D();

  void Resize(float width, float height);

  // 2D Renderer API
  void Begin(Camera *camera, bool useTransform = false, const glm::mat4& globalTransform = glm::mat4(1.0f));
  void End();

  // Shapes
  void DrawPoint(const glm::vec2 &position, const glm::vec4 &color = glm::vec4(1.0f), float radius = 1.0f);
  void DrawSquare(const glm::vec2 &position, const glm::vec4& color = glm::vec4(1.0f), float size = 1.0f);
  void DrawQuad(const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f));
  void DrawTexturedQuad(const glm::mat4& transform, const glm::vec4& color, ID texture, float tilingFactor = 1.0f);
  
  // Outlines
  void DrawLine(const glm::vec2& pos1, const glm::vec2& pos2, const glm::vec4& color = glm::vec4(1.0f), float thickness = 1.0f);
  void DrawCircle(const glm::vec2& position, const glm::vec4& color = glm::vec4(1.0f), float radius = 1.0f, float thickness = 0.1f);
  void DrawBox(const glm::vec2& position, const glm::vec4& color = glm::vec4(1.0f), float size = 1.0f, float thickness = 0.1f);

private:
  void Flush();

  void GenerateBuffers();
  void GeneratePipelines();
  void GenerateTextures();

  void DestroyBuffers();
  void DestroyPipelines();
  void DestroyTextures();

private:
  // General Rendering Data
  RenderDevice* device = nullptr;
  bool inFrame = false;
  Camera *camera = nullptr;
  float pixelDensity = 1.0f;
  float width, height;

  // Matrix UBO
  ID matrixUBO;

  // Mode
  bool useGlobalTransform = false;
  glm::mat4 globalTransform = glm::mat4(1.0f);

  // Quad Info
  const std::size_t maxQuads = 10000;
  std::size_t numQuads = 0;
  QuadVertex *quadBuffer, *quadBufferHead;
  ID quadVBO, quadIBO;
  ID quadPipeline, quadShader;
  
  // Texture Info
  const std::size_t maxTextures = 15;
  ID whiteTexture;
  std::vector<ID> textures;
  std::size_t numUserTextures = 0;

  // Point Info
  const std::size_t maxPoints = 10000;
  std::size_t numPoints = 0;
  PointVertex *pointBuffer, *pointBufferHead;
  ID pointVBO; // We can use the same ibo because the indices for points are the same
  ID pointPipeline, pointShader;
};

}