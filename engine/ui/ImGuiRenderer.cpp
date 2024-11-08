#include "ImGuiRenderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <iostream>

#include "core/App.h"

#include "renderer/shader/ShaderCompiler.h"

namespace Vision
{

ImGuiRenderer::ImGuiRenderer(RenderDevice* renderDevice, float w, float h, float displayScale)
    : device(renderDevice), width(w), height(h), pixelDensity(displayScale)
{
  // Initialize ImGui
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();

  io.BackendRendererName = "Vision Renderer";
  io.BackendPlatformName = "Vision Engine";

  io.DisplaySize = {width, height};
  io.DisplayFramebufferScale = {displayScale, displayScale};

  // Create our rendering resources
  GenerateBuffers();
  GeneratePipeline();
  GenerateTexture();
}

ImGuiRenderer::~ImGuiRenderer()
{
  // Shutdown ImGui
  ImGui::DestroyContext();

  // Destroy our rendering resources
  DestroyBuffers();
  DestroyPipeline();
  DestroyTexture();
}

void ImGuiRenderer::Begin()
{
  ImGui::NewFrame();
}

void ImGuiRenderer::End()
{
  ImGui::Render();
  ImDrawData* drawData = ImGui::GetDrawData();

  // Don't render if the window is minimized
  int32_t fbWidth = static_cast<int32_t>(drawData->DisplaySize.x * drawData->FramebufferScale.x);
  int32_t fbHeight = static_cast<int32_t>(drawData->DisplaySize.y * drawData->FramebufferScale.y);
  if (fbWidth <= 0 || fbHeight <= 0)
  {
    std::cout << "Error! Zero/Negative UI Framebuffer Size" << std::endl;
    return;
  }

  // bind our projection matrix
  float L = drawData->DisplayPos.x;
  float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
  float T = drawData->DisplayPos.y;
  float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
  glm::mat4 projection = glm::ortho(L, R, B, T);
  device->SetBufferData(ubo, &projection, sizeof(glm::mat4));
  device->BindBuffer(ubo);

  // Calculate our clip rect offset and scale
  glm::vec2 clipOff = {drawData->DisplayPos.x, drawData->DisplayPos.y};
  glm::vec2 clipScale = {drawData->FramebufferScale.x, drawData->FramebufferScale.y};

  // Reset our VBO and IBO offsets.
  vboOffset = 0;
  iboOffset = 0;

  // Ensure that our VBO and IBO are large enough.
  if (maxVertices < drawData->TotalVtxCount)
  {
    while (maxVertices < drawData->TotalVtxCount)
      maxVertices *= 2;

    device->ResizeBuffer(vbo, maxVertices * sizeof(ImDrawVert));
  }

  if (maxIndices < drawData->TotalIdxCount)
  {
    while (maxVertices < drawData->TotalIdxCount)
      maxIndices *= 2;

    device->ResizeBuffer(vbo, maxIndices * sizeof(ImDrawIdx));
  }

  for (std::size_t i = 0; i < drawData->CmdListsCount; i++)
  {
    const ImDrawList* cmdList = drawData->CmdLists[i];

    // Copy the draw data into our buffers at the offsets.
    device->SetBufferData(vbo, cmdList->VtxBuffer.Data,
                          cmdList->VtxBuffer.Size * sizeof(ImDrawVert), vboOffset);
    device->SetBufferData(ibo, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx),
                          iboOffset);

    // Prepare a draw command that we can reuse
    DrawCommand drawCmd;
    drawCmd.VertexBuffers = {vbo};
    drawCmd.IndexBuffer = ibo;
    drawCmd.IndexType = sizeof(ImDrawIdx) == 2 ? IndexType::U16 : IndexType::U32;
    drawCmd.RenderPipeline = pipeline;
    drawCmd.Type = PrimitiveType::Triangle;

    // Iterate over each command in the list
    for (std::size_t j = 0; j < cmdList->CmdBuffer.Size; j++)
    {
      const ImDrawCmd* command = &cmdList->CmdBuffer[j];

      // Calculate our scissor rect
      ImVec2 clipMin((command->ClipRect.x - clipOff.x) * clipScale.x,
                     (command->ClipRect.y - clipOff.y) * clipScale.y);
      ImVec2 clipMax((command->ClipRect.z - clipOff.x) * clipScale.x,
                     (command->ClipRect.w - clipOff.y) * clipScale.y);

      // Clamp to viewport as setScissorRect() won't accept values that are off bounds
      if (clipMin.x < 0.0f)
        clipMin.x = 0.0f;
      if (clipMin.y < 0.0f)
        clipMin.y = 0.0f;
      if (clipMax.x > fbWidth)
        clipMax.x = (float)fbWidth;
      if (clipMax.y > fbHeight)
        clipMax.y = (float)fbHeight;
      if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
        continue;
      if (command->ElemCount == 0)
        continue;

      device->SetScissorRect(clipMin.x, clipMin.y, (clipMax.x - clipMin.x),
                             (clipMax.y - clipMin.y));

      if (command->UserCallback)
      {
        command->UserCallback(cmdList, command);
      }
      else
      {
        if (command->TextureId == 0)
          device->BindTexture2D(fontTexture, 0);
        else
          device->BindTexture2D(static_cast<ID>((intptr_t)command->TextureId));

        // Handle our offset into vbo and ibos.
        if (drawCmd.VertexOffsets.empty())
          drawCmd.VertexOffsets.push_back(0);

        drawCmd.VertexOffsets[0] = vboOffset;
        drawCmd.IndexOffset = command->IdxOffset * sizeof(ImDrawIdx) + iboOffset;

        drawCmd.NumVertices = command->ElemCount;
        device->Submit(drawCmd);
      }
    }

    // Add this call to the offset, so the data isn't overwritten until draw call finishes.
    vboOffset += cmdList->VtxBuffer.Size * sizeof(ImDrawVert);
    iboOffset += cmdList->IdxBuffer.Size * sizeof(ImDrawIdx);
  }

  // Disable the scissor rect when we are done rendering
  device->SetScissorRect(0.0f, 0.0f, 0.0f, 0.0f);
}

void ImGuiRenderer::Resize(float w, float h)
{
  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = {w, h};

  width = w * pixelDensity;
  height = h * pixelDensity;
}

void ImGuiRenderer::GenerateBuffers()
{
  BufferDesc vboDesc;
  vboDesc.Type = BufferType::Vertex;
  vboDesc.Usage = BufferUsage::Dynamic;
  vboDesc.Size = sizeof(ImDrawVert) * maxVertices;
  vboDesc.Data = nullptr;
  vboDesc.DebugName = "ImGui Vertex Buffer";
  vbo = device->CreateBuffer(vboDesc);

  BufferDesc iboDesc;
  iboDesc.Type = BufferType::Index;
  iboDesc.Usage = BufferUsage::Dynamic;
  iboDesc.Size = sizeof(ImDrawIdx) * maxIndices;
  iboDesc.Data = nullptr;
  iboDesc.DebugName = "ImGui Index Buffer";
  ibo = device->CreateBuffer(iboDesc);

  BufferDesc uboDesc;
  uboDesc.Type = BufferType::Uniform;
  uboDesc.Usage = BufferUsage::Dynamic;
  uboDesc.Size = sizeof(glm::mat4);
  uboDesc.Data = nullptr;
  uboDesc.DebugName = "ImGui View Projection";
  ubo = device->CreateBuffer(uboDesc);
}

static const char* vertexShader = R"(
#version 450 core

layout (location = 0) in vec2 a_Position;
layout (location = 1) in vec2 a_UV;
layout (location = 2) in vec4 a_Color;

layout (binding = 0) uniform matrices
{
  mat4 u_ViewProjection;
};

out vec4 v_FragColor;
out vec2 v_FragUV;

void main()
{
  v_FragColor = a_Color;
  v_FragUV = a_UV;
  gl_Position = u_ViewProjection * vec4(a_Position, 0.0, 1.0);
})";

static const char* pixelShader = R"(
#version 450 core

in vec4 v_FragColor;
in vec2 v_FragUV;

layout (binding = 0) uniform sampler2D u_Texture;

layout (location = 0) out vec4 f_FragColor;

void main()
{
  f_FragColor = v_FragColor * texture(u_Texture, v_FragUV);
})";

void ImGuiRenderer::GeneratePipeline()
{
  RenderPipelineDesc pipelineDesc;

  // set the layout
  BufferLayout imVertLayout = {
      // ImDrawVert layout
      {ShaderDataType::Float2, "a_Position"},
      {ShaderDataType::Float2, "a_UV"},
      {ShaderDataType::UByte4, "a_Color", true}
  };
  pipelineDesc.Layouts = {imVertLayout};

  // create and set the shader
  ShaderCompiler compiler;
  ShaderSource vertexSource = {ShaderStage::Vertex, "imguiVertex", vertexShader};
  ShaderSource pixelSource = {ShaderStage::Pixel, "imguiPixel", pixelShader};
  pipelineDesc.VertexShader = compiler.CompileSource(vertexSource);
  pipelineDesc.PixelShader = compiler.CompileSource(pixelSource);

  // set the imgui rendering state info
  pipelineDesc.Blending = true;
  pipelineDesc.DepthTest = false;
  pipelineDesc.DepthWrite = false;

  // finally create the pipeline
  pipeline = device->CreateRenderPipeline(pipelineDesc);
}

void ImGuiRenderer::GenerateTexture()
{
  ImGuiIO& io = ImGui::GetIO();

  uint8_t* pixels;
  int32_t width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

  Texture2DDesc textureDesc;
  textureDesc.LoadFromFile = false;
  textureDesc.Width = width;
  textureDesc.Height = height;
  textureDesc.PixelType = PixelType::RGBA8;
  textureDesc.Data = pixels;
  fontTexture = device->CreateTexture2D(textureDesc);
}

void ImGuiRenderer::DestroyBuffers()
{
  device->DestroyBuffer(vbo);
  device->DestroyBuffer(ibo);
  device->DestroyBuffer(ubo);
}

void ImGuiRenderer::DestroyPipeline()
{
  device->DestroyPipeline(pipeline);
}

void ImGuiRenderer::DestroyTexture()
{
  device->DestroyTexture2D(fontTexture);
}

} // namespace Vision
