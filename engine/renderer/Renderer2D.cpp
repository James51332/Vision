#include "Renderer2D.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

namespace Vision
{

constexpr static QuadVertex quadVertices[] = {
  {{-1.0f,  1.0f}, glm::vec4(1.0f), {0.0f, 1.0f}},
  {{ 1.0f,  1.0f}, glm::vec4(1.0f), {1.0f, 1.0f}},
  {{ 1.0f, -1.0f}, glm::vec4(1.0f), {1.0f, 0.0f}},
  {{-1.0f, -1.0f}, glm::vec4(1.0f), {0.0f, 0.0f}}
};

constexpr static uint32_t quadIndices[] = {
  0, 1, 2, 0, 2, 3
};

constexpr static PointVertex pointVertices[] = {
  {{-1.0f, 1.0f}, glm::vec4(1.0f), {0.0f, 1.0f}},
  {{1.0f, 1.0f}, glm::vec4(1.0f), {1.0f, 1.0f}},
  {{1.0f, -1.0f}, glm::vec4(1.0f), {1.0f, 0.0f}},
  {{-1.0f, -1.0f}, glm::vec4(1.0f), {0.0f, 0.0f}}
};

Renderer2D::Renderer2D(RenderDevice* dev, float w, float h, float density)
: device(dev), width(w), height(h), pixelDensity(density)
{
  GenerateBuffers();
  GeneratePipelines();
  GenerateTextures();
}

Renderer2D::~Renderer2D()
{
  DestroyBuffers();
  DestroyPipelines();
  DestroyTextures();
}

void Renderer2D::Resize(float w, float h)
{
  width = w;
  height = h;
}

void Renderer2D::Begin(Camera *cam, bool useTransform, const glm::mat4& transform)
{
  assert(!inFrame);
  inFrame = true;
  camera = cam;

  useGlobalTransform = useTransform;
  globalTransform = transform;
}

void Renderer2D::End()
{
  assert(inFrame);
  Flush();

  inFrame = false;
  camera = nullptr;
}

// Renderer 2D API

void Renderer2D::DrawPoint(const glm::vec2 &position, const glm::vec4 &color, float radius)
{
  DrawCircle(position, color, radius, 0.0f);
}

void Renderer2D::DrawSquare(const glm::vec2 &position, const glm::vec4 &color, float size)
{
  glm::mat4 transform = glm::translate(glm::mat4(1.0f), {position, 0.0f})
                      * glm::scale(glm::mat4(1.0f), { size, size, 1.0f });
  DrawQuad(transform, color);
}

void Renderer2D::DrawQuad(const glm::mat4 &transform, const glm::vec4 &color)
{
  DrawTexturedQuad(transform, color, 0);
}

void Renderer2D::DrawTexturedQuad(const glm::mat4 &transform, const glm::vec4 &color, ID texture, float tilingFactor)
{
  assert(inFrame);

  if (numQuads == maxQuads)
    Flush();
  
  std::size_t textureID = 0; // white texture
  if (texture)
  {
    // look to see if already usiig
    for (std::size_t i = 0; i < numUserTextures; i++)
    {
      if (textures[i] == texture)
      {
        textureID = i;
        break;
      }
    }

    // if no more texture slots, flush
    if (textureID == 0 && numUserTextures == maxTextures)
      Flush();

    textures[numUserTextures] = texture;
    numUserTextures++;
    textureID = numUserTextures;
  }

  // draw the quad by adding the next four vertices to the buffer
  for (std::size_t i = 0; i < 4; ++i)
  {
    QuadVertex vertex = quadVertices[i];
    vertex.Position = transform * glm::vec4(vertex.Position, 0.0f, 1.0f);
    vertex.Color = color;
    vertex.TextureID = textureID;
    vertex.TilingFactor = tilingFactor;

    (*quadBufferHead) = vertex;
    quadBufferHead++;
  }

  numQuads++;
}

// The line renderer converts a line into a quad and two endpoints.
void Renderer2D::DrawLine(const glm::vec2 &pos1, const glm::vec2 &pos2, const glm::vec4& color, float thickness)
{
  assert(inFrame);
  thickness /= 2.0f; // radius and height are half of total

  // basic line data
  glm::vec2 ray = pos2 - pos1;
  glm::vec2 midpoint = pos1 + 0.5f * ray;
  float length = glm::length(ray);

  if (length == 0)
  {
    DrawPoint(pos1, color, thickness);
    return;
  }

  // construct rotation matrix
  float sin = ray.y / length;
  float cos = ray.x / length;
  glm::mat4 rotation = glm::mat2({cos, sin}, {-sin, cos});

  // calculate transform (scale => rotate => translate)
  glm::mat4 transform = glm::translate(glm::mat4(1.0f), { midpoint, 0.0f })
                      * rotation 
                      * glm::scale(glm::mat4(1.0f), { length / 2.0f, thickness, 1.0f });

  DrawQuad(transform, color);
  DrawPoint(pos1, color, thickness);
  DrawPoint(pos2, color, thickness);
}

void Renderer2D::DrawCircle(const glm::vec2 &position, const glm::vec4 &color, float radius, float thickness)
{
  assert(inFrame);

  if (numPoints == maxPoints)
    Flush();

  // The border of circles is on the inside. Therefore new radius = r + thickness / 2
  radius += thickness / 2.0f;
  float border = thickness / radius;

  for (std::size_t i = 0; i < 4; ++i)
  {
    PointVertex vertex = pointVertices[i];
    vertex.Position = vertex.Position * radius + position;
    vertex.Color = color;
    vertex.Border = border;

    (*pointBufferHead) = vertex;
    pointBufferHead++;
  }

  numPoints++;
}

void Renderer2D::DrawBox(const glm::vec2 &position, const glm::vec4 &color, float size, float thickness)
{
  float halfSize = size / 2.0f;
  glm::vec2 topLeft = { position.x - halfSize, position.y + halfSize };
  glm::vec2 topRight = { position.x + halfSize, position.y + halfSize };
  glm::vec2 bottomLeft = { position.x - halfSize, position.y - halfSize };
  glm::vec2 bottomRight = { position.x + halfSize, position.y - halfSize };

  DrawLine(topLeft, topRight, color, thickness);
  DrawLine(topRight, bottomRight, color, thickness);
  DrawLine(bottomRight, bottomLeft, color, thickness);
  DrawLine(bottomLeft, topLeft, color, thickness);
}

void Renderer2D::Flush()
{
  // upload model view projection matrix to GPU
  glm::mat4 mvp;
  if (useGlobalTransform)
    mvp = camera->GetViewProjectionMatrix() * globalTransform;
  else
    mvp = camera->GetViewProjectionMatrix();

  device->SetBufferData(matrixUBO, &mvp[0][0], sizeof(glm::mat4));
  device->AttachUniformBuffer(matrixUBO);

  // Quads
  if (numQuads != 0)
  {
    // copy the quad buffer into the vbo
    device->SetBufferData(quadVBO, quadBuffer, sizeof(QuadVertex) * 4 * numQuads);

    // Bind Textures
    device->BindTexture2D(whiteTexture);
    for (std::size_t i = 1; i <= maxTextures; ++i)
    {
      // HACK: macOS prefers we bind a texture, even if not used.
      ID texture = textures[i] ? textures[i] : whiteTexture;
      device->BindTexture2D(texture, i);
    }

    DrawCommand cmd;
    cmd.Type = PrimitiveType::Triangle;
    cmd.VertexBuffers = { quadVBO };
    cmd.NumVertices = numQuads * 6;
    cmd.IndexType = IndexType::U32;
    cmd.IndexBuffer = quadIBO;
    cmd.Pipeline = quadPipeline;
    device->Submit(cmd);
  }
  
  quadBufferHead = quadBuffer;
  numQuads = 0;
  numUserTextures = 0;

  // Points
  if (numPoints != 0)
  {
    device->SetBufferData(pointVBO, pointBuffer, sizeof(PointVertex) * 4 * numPoints);

    DrawCommand cmd;
    cmd.Type = PrimitiveType::Triangle;
    cmd.VertexBuffers = { pointVBO };
    cmd.NumVertices = numPoints * 6;
    cmd.IndexType = IndexType::U32;
    cmd.IndexBuffer = quadIBO;
    cmd.Pipeline = pointPipeline;
    device->Submit(cmd);
  }

  pointBufferHead = pointBuffer;
  numPoints = 0;
}

void Renderer2D::GenerateBuffers()
{ 
  // Quad Buffers
  {
    std::size_t numVertices = maxQuads * 4; // 4 vertices per quad
    std::size_t numIndices = maxQuads * 6; // 6 indices per quad

    quadBuffer = new QuadVertex[numVertices];
    quadBufferHead = quadBuffer;

    BufferDesc vboDesc;
    vboDesc.Type = BufferType::Vertex;
    vboDesc.Usage = BufferUsage::Dynamic;
    vboDesc.Size = sizeof(QuadVertex) * numVertices;
    vboDesc.Data = nullptr;
    vboDesc.DebugName = "Renderer2D Quad VBO";
    quadVBO = device->CreateBuffer(vboDesc);

    // Populate index buffer
    std::vector<std::uint32_t> indices(numIndices);
    for (std::size_t i = 0; i < numIndices; ++i)
    {
      std::size_t quad = i / 6; // integer division
      indices[i] = (quad * 4) + quadIndices[i % 6];
    }

    BufferDesc iboDesc;
    iboDesc.Type = BufferType::Index;
    iboDesc.Usage = BufferUsage::Static;
    iboDesc.Size = sizeof(std::uint32_t) * numIndices;
    iboDesc.Data = indices.data();
    iboDesc.DebugName = "Renderer2D IBO";
    quadIBO = device->CreateBuffer(iboDesc);
  }

  // Point Buffers
  {
    std::size_t numVertices = maxPoints * 4; // 4 vertices per point
    std::size_t numIndices = maxPoints * 6;  // 6 indices per point

    pointBuffer = new PointVertex[numVertices];
    pointBufferHead = pointBuffer;

    BufferDesc vboDesc;
    vboDesc.Type = BufferType::Vertex;
    vboDesc.Usage = BufferUsage::Dynamic;
    vboDesc.Size = sizeof(PointVertex) * numVertices;
    vboDesc.Data = nullptr;
    vboDesc.DebugName = "Renderer2D Point VBO";
    pointVBO = device->CreateBuffer(vboDesc);
  }

  // UBO
  {
    BufferDesc uboDesc;
    uboDesc.Type = BufferType::Uniform;
    uboDesc.Usage = BufferUsage::Dynamic;
    uboDesc.Size = sizeof(glm::mat4);
    uboDesc.Data = nullptr;
    matrixUBO = device->CreateBuffer(uboDesc);
  }
}

const char *quadVertex = R"(
#version 450 core

layout (location = 0) in vec2 a_Position;
layout (location = 1) in vec4 a_Color;
layout (location = 2) in vec2 a_UV;
layout (location = 3) in int a_TextureID;
layout (location = 4) in float a_TilingFactor;

out vec2 v_UV;
out vec4 v_Color;
flat out int v_TextureID;

layout (binding = 0) uniform matrices 
{
  mat4 mvp;
};

void main()
{  
  gl_Position = mvp * vec4(a_Position, 0.0, 1.0);
  
  v_UV = a_UV * a_TilingFactor;
  v_Color = a_Color;
  v_TextureID = a_TextureID;
})";

const char *quadPixel = R"(
#version 450 core

in vec2 v_UV;
in vec4 v_Color;
flat in int v_TextureID;

out vec4 FragColor;

layout (binding = 0) uniform sampler2D u_Textures[16];

void main()
{
  FragColor = v_Color * texture(u_Textures[v_TextureID], v_UV.st);
})";

const char *pointVertex = R"(
#version 450 core

layout (location = 0) in vec2 a_Position;
layout (location = 1) in vec4 a_Color;
layout (location = 2) in vec2 a_UV;
layout (location = 3) in float a_Border;

out vec2 v_UVNorm;
out vec4 v_Color;
out float v_Border;

layout (binding = 0) uniform matrices 
{
  mat4 mvp;
};

void main()
{  
  gl_Position = mvp * vec4(a_Position, 0.0, 1.0);

  v_Color = a_Color;
  v_UVNorm = a_UV * 2.0 - 1.0; // normalize from -1 to 1
  v_Border = a_Border;
})";

const char *pointPixel = R"(
#version 450 core

in vec2 v_UVNorm;
in vec4 v_Color;
in float v_Border;

out vec4 FragColor;

void main()
{
  float dist = length(v_UVNorm);
  float radius = 1.0;

	float delta = fwidth(dist);
	float outerFade = smoothstep(radius + delta, radius - delta, dist);
  float innerFade = 1.0 - float(v_Border > 0) * smoothstep(radius - v_Border + delta, radius - v_Border - delta, dist);
  float alpha = min(outerFade, innerFade);

	FragColor = vec4(v_Color.xyz, v_Color.w * alpha);
})";

void Renderer2D::GeneratePipelines()
{
  // Quads
  ShaderDesc quadShaderDesc;
  quadShaderDesc.Source = ShaderSource::GLSL;
  quadShaderDesc.StageMap[ShaderStage::Vertex] = quadVertex;
  quadShaderDesc.StageMap[ShaderStage::Pixel] = quadPixel;
  quadShader = device->CreateShader(quadShaderDesc);

  PipelineDesc quadDesc;
  quadDesc.Layouts = { BufferLayout({
    { ShaderDataType::Float2, "a_Position" },
    { ShaderDataType::Float4, "a_Color" },
    { ShaderDataType::Float2, "a_UV" },
    { ShaderDataType::Int, "a_TextureID"},
    { ShaderDataType::Float, "a_TilingFactor" }
  })};
  quadDesc.DepthTest = false;
  quadDesc.DepthWrite = false;
  quadDesc.Blending = true;
  quadDesc.Shader = quadShader;
  quadPipeline = device->CreatePipeline(quadDesc);

  // Point
  ShaderDesc pointShaderDesc;
  pointShaderDesc.Source = ShaderSource::GLSL;
  pointShaderDesc.StageMap[ShaderStage::Vertex] = pointVertex;
  pointShaderDesc.StageMap[ShaderStage::Pixel] = pointPixel;
  pointShader = device->CreateShader(pointShaderDesc);

  PipelineDesc pointDesc;
  pointDesc.Layouts = {BufferLayout( {
                        { ShaderDataType::Float2, "a_Position" },
                        { ShaderDataType::Float4, "a_Color" },
                        { ShaderDataType::Float2, "a_UV" },
                        { ShaderDataType::Float, "a_Border" },
                      })};
  pointDesc.DepthTest = false;
  pointDesc.DepthWrite = false;
  pointDesc.Blending = true;
  pointDesc.Shader = pointShader;
  pointPipeline = device->CreatePipeline(pointDesc);
}

void Renderer2D::GenerateTextures()
{
  std::uint32_t data = 0xffffffff;
  Texture2DDesc desc;
  desc.LoadFromFile = false;
  desc.PixelType = PixelType::RGBA32;
  desc.Width = 1;
  desc.Height = 1;
  desc.Data =(uint8_t*)(&data);
  desc.WriteOnly = false;
  
  whiteTexture = device->CreateTexture2D(desc);
  textures = std::vector<ID>(maxTextures, 0);
}

void Renderer2D::DestroyBuffers()
{
  delete[] quadBuffer;
  device->DestroyBuffer(quadVBO);
  device->DestroyBuffer(quadIBO);

  delete[] pointBuffer;
  device->DestroyBuffer(pointVBO);
}

void Renderer2D::DestroyPipelines()
{
  device->DestroyPipeline(quadPipeline);
  device->DestroyPipeline(pointPipeline);
  
  device->DestroyShader(quadShader);
  device->DestroyShader(pointShader);
}

void Renderer2D::DestroyTextures()
{
  device->DestroyTexture2D(whiteTexture);
}

}