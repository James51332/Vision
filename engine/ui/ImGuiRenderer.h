#pragma once

#include <imgui.h>

#include <glad/glad.h>

#include "renderer/RenderDevice.h"
#include "renderer/primitive/Buffer.h"
#include "renderer/primitive/Shader.h"
#include "renderer/opengl/GLVertexArray.h"

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

  ID vbo, ibo, ubo;
  ID pipeline;
  ID fontTexture;

  std::size_t maxVertices = 1000000; // 5MB GPU buffer
  std::size_t maxIndices = 1000000;

  float pixelDensity = 1.0f;
  float width, height;
};

}