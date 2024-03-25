#include "Renderer2D.h"

#include <glm/glm.hpp>
#include <glad/glad.h>

namespace Vision
{

struct QuadVertex
{
  glm::vec3 Position;
  glm::vec2 UV;
};

constexpr static QuadVertex quadVertices[] = {
  {{-1.0f,  1.0f, 0.0f}, {0.0f, 1.0f}},
  {{ 1.0f,  1.0f, 0.0f}, {1.0f, 1.0f}},
  {{ 1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
  {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}
};

constexpr static uint16_t quadIndices[] = {
  0, 1, 2, 0, 2, 3
};

const char* particleVertex = R"(
#version 410 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_UV;
layout (location = 2) in vec2 a_InstancePosition;
layout (location = 3) in vec4 a_InstanceColor;
layout (location = 4) in float a_InstanceRadius;

out vec2 v_UVNorm;
out vec4 v_Color;

uniform mat4 u_ViewProjection;

void main()
{  
  gl_Position = u_ViewProjection * vec4(a_Position.xy * a_InstanceRadius + a_InstancePosition, 0.0f, 1.0f);
  v_UVNorm = a_UV * 2.0f - 1.0f;
  v_Color = a_InstanceColor;
})";

const char* particleFragment = R"(
#version 410 core

in vec2 v_UVNorm;
in vec4 v_Color;

out vec4 FragColor;

void main()
{
  float dist = distance(vec2(0.0f), v_UVNorm);
  float radius = 1.0f;

	float delta = fwidth(dist);
	float alpha = smoothstep(radius + delta, radius - delta, dist);
	FragColor = vec4(v_Color.xyz, v_Color.w * alpha);
})";

const char* quadVertex = R"(
#version 410 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_UV;

layout (location = 0) out vec2 v_UV;

void main()
{  
  gl_Position = vec4(a_Position, 1.0);
  v_UV = a_UV;
})";

const char *quadFragment = R"(
#version 410 core

layout (location = 0) in vec2 v_UV;

out vec4 FragColor;

uniform sampler2D u_Texture;

void main()
{
  FragColor = texture(u_Texture, v_UV);
})";

Renderer2D::Renderer2D(float width, float height, float pixelDensity)
: m_Width(width), m_Height(height), m_PixelDensity(pixelDensity)
{
  GenerateBuffers();
  GenerateArrays();
  GenerateShaders();
}

Renderer2D::~Renderer2D()
{
  DestroyArrays();
  DestroyBuffers();
  DestroyShaders();
}

void Renderer2D::Resize(float width, float height)
{
  m_Width = width;
  m_Height = height;

  glViewport(0, 0, static_cast<GLsizei>(width * m_PixelDensity), static_cast<GLsizei>(height * m_PixelDensity));
}

void Renderer2D::Begin(OrthoCamera *camera)
{
  assert(!m_InFrame);

  m_InFrame = true;
  m_Camera = camera;

  // Enable Blending and Disable Depth Testing
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);
}

void Renderer2D::End()
{
  assert(m_InFrame);
  Flush();

  m_InFrame = false;
  m_Camera = nullptr;
}

void Renderer2D::DrawPoint(const glm::vec2 &point, const glm::vec4 &color, float radius)
{
  assert(m_InFrame);

  if (m_Points == m_MaxPoints)
    Flush();

  m_InstancedBuffer[m_Points] = {point, color, radius};
  m_Points++;
}

void Renderer2D::DrawFullscreenQuad(const Texture2D* texture)
{  
  
  m_QuadShader->Use();
  m_QuadShader->UploadUniformInt(0, "u_Texture");
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture->m_TextureID);
  
  m_QuadVAO->Bind();
  m_QuadIBO->Bind();
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

void Renderer2D::Flush()
{
  // Upload the camera matrix and use the shader program
  m_PointShader->Use();
  m_PointShader->UploadUniformMat4(&m_Camera->GetViewProjectionMatrix()[0][0], "u_ViewProjection");

  // Copy the instanced buffers to the instanced vbos
  m_InstancedVBO->SetData(m_InstancedBuffer, sizeof(InstancedVertex) * m_Points);

  // Display the instances
  m_PointsVAO->Bind();
  m_QuadIBO->Bind();
  glDrawElementsInstanced(GL_TRIANGLES, sizeof(quadIndices) / sizeof(quadIndices[0]), GL_UNSIGNED_SHORT, nullptr, m_Points);

  // Reset the point count
  m_Points = 0;
}

void Renderer2D::GenerateBuffers()
{
  // Create our quad vertex buffer and index buffer
  {
    Vision::BufferDesc vboDesc;
    vboDesc.Type = GL_ARRAY_BUFFER;
    vboDesc.Usage = GL_STATIC_DRAW;
    vboDesc.Size = sizeof(quadVertices);
    vboDesc.Data = (void *)quadVertices;
    vboDesc.Layout = {
      { Vision::ShaderDataType::Float3, "a_Position" },
      { Vision::ShaderDataType::Float2, "a_UV" }
    };

    m_QuadVBO = new Vision::Buffer(vboDesc);

    Vision::BufferDesc iboDesc;
    iboDesc.Type = GL_ELEMENT_ARRAY_BUFFER;
    iboDesc.Usage = GL_STATIC_DRAW;
    iboDesc.Size = sizeof(quadIndices);
    iboDesc.Data = (void *)quadIndices;

    m_QuadIBO = new Vision::Buffer(iboDesc);
  }

  // Create our particle instance vertex buffers
  {
    BufferDesc instancedDesc;
    instancedDesc.Type = GL_ARRAY_BUFFER;
    instancedDesc.Usage = GL_DYNAMIC_DRAW;
    instancedDesc.Size = m_MaxPoints * sizeof(InstancedVertex);
    instancedDesc.Data = nullptr;
    instancedDesc.Layout = {
        {ShaderDataType::Float2, "a_InstancedPosition", false, 1},
        {ShaderDataType::Float4, "a_InstancedColor", false, 1},
        {ShaderDataType::Float, "a_InstanceRadius", false, 1}};

    m_InstancedVBO = new Buffer(instancedDesc);

    // Allocate the cpu buffer that we'll write to and copy from
    m_InstancedBuffer = new InstancedVertex[m_MaxPoints];
  }
}

void Renderer2D::GenerateArrays()
{
  // Create our particle vertex array and attach buffers
  m_PointsVAO = new VertexArray();
  m_PointsVAO->AttachBuffer(m_QuadVBO);
  m_PointsVAO->AttachBuffer(m_InstancedVBO);

  m_QuadVAO = new VertexArray();
  m_QuadVAO->AttachBuffer(m_QuadVBO);
}

void Renderer2D::GenerateShaders()
{
  m_PointShader = new Shader(particleVertex, particleFragment);
  m_QuadShader = new Shader(quadVertex, quadFragment);
}

void Renderer2D::DestroyBuffers()
{
  delete m_InstancedVBO;
  delete[] m_InstancedBuffer;
}

void Renderer2D::DestroyArrays()
{
  delete m_PointsVAO;
  delete m_QuadVAO;
}

void Renderer2D::DestroyShaders()
{
  delete m_PointShader;
  delete m_QuadShader;
}

}