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

Renderer2D::Renderer2D(float width, float height, float pixelDensity)
: m_Width(width), m_Height(height), m_PixelDensity(pixelDensity)
{
  GenerateBuffers();
  GenerateArrays();
  GenerateShaders();
  GenerateTextures();
}

Renderer2D::~Renderer2D()
{
  DestroyArrays();
  DestroyBuffers();
  DestroyShaders();
  DestroyTextures();
}

void Renderer2D::Resize(float width, float height)
{
  m_Width = width;
  m_Height = height;

  glViewport(0, 0, static_cast<GLsizei>(width * m_PixelDensity), static_cast<GLsizei>(height * m_PixelDensity));
}

void Renderer2D::Begin(Camera *camera)
{
  assert(!m_InFrame);

  m_InFrame = true;
  m_Camera = camera;

  // Enable Blending and Disable Depth Testing
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_DEPTH_TEST);
}

void Renderer2D::End()
{
  assert(m_InFrame);
  Flush();

  m_InFrame = false;
  m_Camera = nullptr;
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
  DrawTexturedQuad(transform, color, nullptr);
}

void Renderer2D::DrawTexturedQuad(const glm::mat4 &transform, const glm::vec4 &color, Texture2D *texture, float tilingFactor)
{
  assert(m_InFrame);

  if (m_NumQuads == m_MaxQuads)
    Flush();
  
  std::size_t textureID = 0; // white texture
  if (texture)
  {
    // look to see if already usiig
    for (std::size_t i = 0; i < m_NumTextures; i++)
    {
      if (m_Textures[i]->m_TextureID == texture->m_TextureID)
      {
        textureID = i;
        break;
      }
    }

    // if no more texture slots, flush
    if (textureID == 0 && m_NumTextures == m_MaxTextures)
      Flush();

    m_Textures[m_NumTextures] = texture;
    textureID = m_NumTextures;
    m_NumTextures++;
  }

  // draw the quad by adding the next four vertices to the buffer
  for (std::size_t i = 0; i < 4; ++i)
  {
    QuadVertex vertex = quadVertices[i];
    vertex.Position = transform * glm::vec4(vertex.Position, 0.0f, 1.0f);
    vertex.Color = color;
    vertex.TextureID = textureID;
    vertex.TilingFactor = tilingFactor;

    (*m_QuadBufferHead) = vertex;
    m_QuadBufferHead++;
  }

  m_NumQuads++;
}

// The line renderer converts a line into a quad and two endpoints.
void Renderer2D::DrawLine(const glm::vec2 &pos1, const glm::vec2 &pos2, const glm::vec4& color, float thickness)
{
  assert(m_InFrame);

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
  assert(m_InFrame);

  if (m_NumPoints == m_MaxPoints)
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

    (*m_PointBufferHead) = vertex;
    m_PointBufferHead++;
  }

  m_NumPoints++;
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
  // TODO: Right now, the most recent quads will draw over each other, and the points will draw over quads
  // maybe some form a z-value to sort by draw order? We could have an API to push layer to the render 2D.
  glDisable(GL_DEPTH_TEST);

  // Quads
  if (m_NumQuads != 0)
  {
    // copy the quad buffer into the vbo
    m_QuadVBO->SetData(m_QuadBuffer, sizeof(QuadVertex) * 4 * m_NumQuads);

    m_QuadShader->Use();
    m_QuadShader->UploadUniformMat4(&m_Camera->GetViewProjectionMatrix()[0][0], "u_ViewProjection");

    // Bind Textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_WhiteTexture->m_TextureID);

    for (std::size_t i = 1; i <= m_MaxTextures; ++i)
    {
      // HACK: macOS prefers we bind a texture, even if not used.
      Texture2D *texture = m_Textures[i] ? m_Textures[i] : m_WhiteTexture;
      glActiveTexture(GL_TEXTURE0 + i);
      glBindTexture(GL_TEXTURE_2D, texture->m_TextureID);
    }

    m_QuadVAO->Bind();
    m_QuadIBO->Bind();
    glDrawElements(GL_TRIANGLES, m_NumQuads * 6, GL_UNSIGNED_INT, nullptr);
  }
  
  m_QuadBufferHead = m_QuadBuffer;
  m_NumQuads = 0;
  m_NumTextures = 1;

  // Points
  if (m_NumPoints != 0)
  {
    m_PointVBO->SetData(m_PointBuffer, sizeof(PointVertex) * 4 * m_NumPoints);

    m_PointShader->Use();
    m_PointShader->UploadUniformMat4(&m_Camera->GetViewProjectionMatrix()[0][0], "u_ViewProjection");

    m_PointVAO->Bind();
    m_QuadIBO->Bind();
    glDrawElements(GL_TRIANGLES, m_NumPoints * 6, GL_UNSIGNED_INT, nullptr);
  }

  m_PointBufferHead = m_PointBuffer;
  m_NumPoints = 0;
}

void Renderer2D::GenerateBuffers()
{ 
  // Quad Buffers
  {
    std::size_t numVertices = m_MaxQuads * 4; // 4 vertices per quad
    std::size_t numIndices = m_MaxQuads * 6; // 6 indices per quad

    m_QuadBuffer = new QuadVertex[numVertices];
    m_QuadBufferHead = m_QuadBuffer;

    BufferDesc vboDesc;
    vboDesc.Type = GL_ARRAY_BUFFER;
    vboDesc.Usage = GL_DYNAMIC_DRAW;
    vboDesc.Size = sizeof(QuadVertex) * numVertices;
    vboDesc.Data = nullptr;
    vboDesc.Layout = {
      { ShaderDataType::Float2, "a_Position" },
      { ShaderDataType::Float4, "a_Color" },
      { ShaderDataType::Float2, "a_UV" },
      { ShaderDataType::Int, "a_TextureID"},
      { ShaderDataType::Float, "a_TilingFactor" }
    };
    m_QuadVBO = new Buffer(vboDesc);

    // Populate index buffer
    std::vector<std::uint32_t> indices(numIndices);
    for (std::size_t i = 0; i < numIndices; ++i)
    {
      std::size_t quad = i / 6; // integer division
      indices[i] = (quad * 4) + quadIndices[i % 6];
    }

    BufferDesc iboDesc;
    iboDesc.Type = GL_ELEMENT_ARRAY_BUFFER;
    iboDesc.Usage = GL_STATIC_DRAW;
    iboDesc.Size = sizeof(std::uint32_t) * numIndices;
    iboDesc.Data = indices.data();
    m_QuadIBO = new Buffer(iboDesc);
  }

  // Point Buffers
  {
    std::size_t numVertices = m_MaxPoints * 4; // 4 vertices per point
    std::size_t numIndices = m_MaxPoints * 6;  // 6 indices per point

    m_PointBuffer = new PointVertex[numVertices];
    m_PointBufferHead = m_PointBuffer;

    BufferDesc vboDesc;
    vboDesc.Type = GL_ARRAY_BUFFER;
    vboDesc.Usage = GL_DYNAMIC_DRAW;
    vboDesc.Size = sizeof(PointVertex) * numVertices;
    vboDesc.Data = nullptr;
    vboDesc.Layout = {
      { ShaderDataType::Float2, "a_Position" },
      { ShaderDataType::Float4, "a_Color" },
      { ShaderDataType::Float2, "a_UV" },
      { ShaderDataType::Float, "a_Border" },
    };
    m_PointVBO = new Buffer(vboDesc);
  }
}

void Renderer2D::GenerateArrays()
{
  // Quads
  m_QuadVAO = new VertexArray();
  m_QuadVAO->AttachBuffer(m_QuadVBO);

  // Point
  m_PointVAO = new VertexArray();
  m_PointVAO->AttachBuffer(m_PointVBO);
}

const char *quadVertex = R"(
#version 410 core

layout (location = 0) in vec2 a_Position;
layout (location = 1) in vec4 a_Color;
layout (location = 2) in vec2 a_UV;
layout (location = 3) in int a_TextureID;
layout (location = 4) in float a_TilingFactor;

out vec2 v_UV;
out vec4 v_Color;
flat out int v_TextureID;

uniform mat4 u_ViewProjection;

void main()
{  
  gl_Position = u_ViewProjection * vec4(a_Position, 0.0, 1.0);
  
  v_UV = a_UV * a_TilingFactor;
  v_Color = a_Color;
  v_TextureID = a_TextureID;
})";

const char *quadFragment = R"(
#version 410 core

in vec2 v_UV;
in vec4 v_Color;
flat in int v_TextureID;

out vec4 FragColor;

uniform sampler2D u_Textures[16];

void main()
{
  FragColor = v_Color * texture(u_Textures[v_TextureID], v_UV.st);
})";

const char *pointVertex = R"(
#version 410 core

layout (location = 0) in vec2 a_Position;
layout (location = 1) in vec4 a_Color;
layout (location = 2) in vec2 a_UV;
layout (location = 3) in float a_Border;

out vec2 v_UVNorm;
out vec4 v_Color;
out float v_Border;

uniform mat4 u_ViewProjection;

void main()
{  
  gl_Position = u_ViewProjection * vec4(a_Position, 0.0, 1.0);

  v_Color = a_Color;
  v_UVNorm = a_UV * 2.0 - 1.0; // normalize from -1 to 1
  v_Border = a_Border;
})";

const char *pointFragment = R"(
#version 410 core

in vec4 v_Color;
in vec2 v_UVNorm;
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

void Renderer2D::GenerateShaders()
{
  m_QuadShader = new Shader(quadVertex, quadFragment);
  
  static std::int32_t textures[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
  m_QuadShader->Use();
  m_QuadShader->UploadUniformIntArray(textures, 16, "u_Textures");

  m_PointShader = new Shader(pointVertex, pointFragment);
}

void Renderer2D::GenerateTextures()
{
  std::uint32_t data = 0xffffffff;
  m_WhiteTexture = new Texture2D(1, 1, PixelType::RGBA32);
  m_WhiteTexture->SetData((uint8_t*)&data);

  m_Textures = std::vector<Texture2D*>(m_MaxTextures, nullptr);
}

void Renderer2D::DestroyBuffers()
{
  delete[] m_QuadBuffer;
  delete m_QuadVBO;
  delete m_QuadIBO;

  delete[] m_PointBuffer;
  delete m_PointVBO;
}

void Renderer2D::DestroyArrays()
{
  delete m_QuadVAO;
  delete m_PointVAO;
}

void Renderer2D::DestroyShaders()
{
  delete m_QuadShader;
  delete m_PointShader;
}

void Renderer2D::DestroyTextures()
{
  delete m_WhiteTexture;
}

}