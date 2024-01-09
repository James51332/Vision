#include "Renderer.h"

#include <SDL.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

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

const char* backgroundVertex = R"(
#version 410 core

layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_UV;

out vec2 v_UV;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

void main()
{
	v_UV = a_UV;
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Pos, 1.0);
})";

const char* backgroundFragment = R"(
#version 410 core

in vec2 v_UV;

out vec4 FragColor;

void main()
{
	FragColor = vec4(0.1, 0.1, 0.1, 1.0);
})";

Renderer::Renderer(float width, float height, float displayScale)
  : m_Width(width), m_Height(height), m_PixelDensity(displayScale)
{
  // Load OpenGL function pointers
  gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

  // Init OpenGL objects
  GenerateBuffers();
  GenerateArrays();
  GenerateShaders();

  // Resize the viewport (no need to use Resize() because we've already done everything else it does)
  glViewport(0, 0, static_cast<GLsizei>(width * displayScale), static_cast<GLsizei>(height * displayScale));
  
  // Enable Blending and Depth Testing
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_DEPTH_TEST);
}

Renderer::~Renderer()
{
  // Destroy rendering objects
  DestroyBuffers();
  DestroyArrays();
  DestroyShaders();
}

void Renderer::BeginFrame(OrthoCamera* camera, float systemBoundSize)
{
  SDL_assert(!m_InFrame && !m_InScene);

  m_InFrame = true;
  m_Camera = camera;
  
  m_BackgroundShader->Use();
  m_BackgroundShader->UploadUniformMat4(&m_Camera->GetViewProjectionMatrix()[0][0], "u_ViewProjection");

  glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(systemBoundSize, systemBoundSize, 1.0f));
  m_BackgroundShader->UploadUniformMat4(&transform[0][0], "u_Transform");
  
  m_BackgroundVAO->Bind();
  m_QuadIBO->Bind();
  glDrawElements(GL_TRIANGLES, sizeof(quadIndices) / sizeof(quadIndices[0]), GL_UNSIGNED_SHORT, nullptr);
}

void Renderer::BeginScene(PerspectiveCamera* camera)
{
  SDL_assert(!m_InFrame && !m_InScene);

  m_InScene = true;
  m_SceneCamera = camera;
}

void Renderer::DrawPoint(const glm::vec2& point, const glm::vec4& color, float radius)
{
  assert(m_InFrame);

  if (m_Points == m_MaxPoints)
    Flush();
  
  m_InstancedBuffer[m_Points] = { point, color, radius };
  m_Points++;
}

void Renderer::EndFrame()
{
  assert(m_InFrame);
  Flush();
  
  m_InFrame = false;
  m_Camera = nullptr;
}

void Renderer::EndScene()
{
  assert(m_InScene);

  m_InScene = false;
  m_SceneCamera = nullptr;
}

void Renderer::DrawMesh(Mesh* mesh, Shader* shader)
{
  assert(m_InScene);

  shader->Use();
  shader->UploadUniformMat4(&m_SceneCamera->GetViewProjectionMatrix()[0][0], "u_ViewProjection");

  mesh->Bind();
  glDrawElements(GL_TRIANGLES, mesh->GetNumIndices(), GL_UNSIGNED_SHORT, nullptr);
}

void Renderer::Flush()
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

void Renderer::Resize(float width, float height)
{
  m_Width = width;
  m_Height = height;

  glViewport(0, 0, static_cast<GLsizei>(width * m_PixelDensity), static_cast<GLsizei>(height * m_PixelDensity));
}

void Renderer::GenerateBuffers()
{
  // Create our quad vertex buffer and index buffer
  {
    BufferDesc vboDesc;
    vboDesc.Type = GL_ARRAY_BUFFER;
    vboDesc.Usage = GL_STATIC_DRAW;
    vboDesc.Size = sizeof(quadVertices);
    vboDesc.Data = (void*)quadVertices;
    vboDesc.Layout = {
      {ShaderDataType::Float3, "a_Position"},
      {ShaderDataType::Float2, "a_UV"},
    };

    m_QuadVBO = new Buffer(vboDesc);

    BufferDesc iboDesc;
    iboDesc.Type = GL_ELEMENT_ARRAY_BUFFER;
    iboDesc.Usage = GL_STATIC_DRAW;
    iboDesc.Size = sizeof(quadIndices);
    iboDesc.Data = (void*)quadIndices;

    m_QuadIBO = new Buffer(iboDesc);
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
        {ShaderDataType::Float, "a_InstanceRadius", false, 1}
    };

    m_InstancedVBO = new Buffer(instancedDesc);
  	
  	// Allocate the cpu buffer that we'll write to and copy from
    m_InstancedBuffer = new InstancedVertex[m_MaxPoints];
	}
}

void Renderer::GenerateArrays()
{
  // Create our particle vertex array and attach buffers
  m_PointsVAO = new VertexArray();
  m_PointsVAO->AttachBuffer(m_QuadVBO);
  m_PointsVAO->AttachBuffer(m_InstancedVBO);
  
  // Create our background vertex array
  m_BackgroundVAO = new VertexArray();
  m_BackgroundVAO->AttachBuffer(m_QuadVBO);
}

void Renderer::GenerateShaders()
{
  m_PointShader = new Shader(particleVertex, particleFragment);
  m_BackgroundShader = new Shader(backgroundVertex, backgroundFragment);
}

void Renderer::DestroyBuffers()
{
  delete m_QuadVBO;
  delete m_QuadIBO;
  delete m_InstancedVBO;
  delete[] m_InstancedBuffer;
}

void Renderer::DestroyArrays()
{
  delete m_PointsVAO;
  delete m_BackgroundVAO;
}

void Renderer::DestroyShaders()
{
  delete m_PointShader;
  delete m_BackgroundShader;
}

}
