#pragma once

#include "renderer/RenderDevice.h"

namespace Vision
{

class ImGuiRenderer
{
public:
  ImGuiRenderer(RenderDevice* device, float width, float height, float displayScale = 1.0f);
  ~ImGuiRenderer();

  void Begin();
  void End();

  void Resize(float width, float height);

private:
  void GenerateBuffers();
  void GeneratePipeline();
  void GenerateTexture();

  void DestroyBuffers();
  void DestroyPipeline();
  void DestroyTexture();

private:
  RenderDevice* device;

  // We cannot update these until the draw calls have finished.
  std::size_t maxVertices = 100000; // 500 kilobyte GPU buffer
  std::size_t maxIndices = 100000;
  std::size_t maxBuffers = 4;
  std::size_t vboOffset = 0, iboOffset = 0;
  ID vbo, ibo;

  // Other Renderer Data.
  ID ubo;
  ID pipeline;
  ID fontTexture;

  float pixelDensity = 1.0f;
  float width, height;
};

} // namespace Vision