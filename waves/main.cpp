#include "core/App.h"

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <imgui.h>
#include <vector>
#include <glm/gtx/rotate_vector.hpp>

#include "renderer/Camera.h"
#include "renderer/Renderer.h"
#include "renderer/Mesh.h"
#include "renderer/MeshGenerator.h"
#include "renderer/Shader.h"

#include "ui/ImGuiRenderer.h"

struct Wave
{
  glm::vec2 origin;
  glm::vec2 direction;
  glm::vec4 scale; // amplitude, wavelength, angular frequency, phase
};

class Waves : public Vision::App
{
public:
  Waves()
  {
    // Initialize the renderer
    m_Renderer = new Vision::Renderer(m_DisplayWidth, m_DisplayHeight, m_DisplayScale);
    m_UIRenderer = new Vision::ImGuiRenderer(m_DisplayWidth, m_DisplayHeight, m_DisplayScale);
    m_PerspectiveCamera = new Vision::PerspectiveCamera(m_DisplayWidth, m_DisplayHeight);

    GenerateWaves();
    GenerateMesh();
    GenerateShader();
  }

  ~Waves()
  {
    delete m_Mesh;
    delete m_Renderer;
    delete m_UIRenderer;
    delete m_PerspectiveCamera;
  }

  void OnUpdate(float timestep)
  {
    // Update Camera Controller
    m_PerspectiveCamera->Update(timestep);

    // Render
    glClearColor(0.776f, 0.998f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static int waves = 100;
    m_Renderer->Begin(m_PerspectiveCamera);
    {
      m_WaterShader->Use();
      m_WaterShader->UploadUniformInt(waves, "u_Waves");
      m_Renderer->DrawMesh(m_Mesh, m_WaterShader, glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), {1.0f, 0.0f, 0.0f}));
    }
    m_Renderer->End();

    m_UIRenderer->Begin();
    {
      ImGui::Begin("Waves");
      ImGui::DragInt("Num Waves", &waves, 1.0, 1, 100);
      ImGui::End();
    }
    m_UIRenderer->End();
  }

  void OnResize()
  {
    m_Renderer->Resize(m_DisplayWidth, m_DisplayHeight);
    m_UIRenderer->Resize(m_DisplayWidth, m_DisplayHeight);
    m_PerspectiveCamera->SetWindowSize(m_DisplayWidth, m_DisplayHeight);
  }

private:
  void GenerateWaves()
  {
    srand(time(0));

    float frequency = 5.0f;
    float amplitude = 4.0f; 
    float wavelength = 10.0f; 
    for (int i = 0; i < m_NumWaves; i++)
    {
      Wave& wave = m_Waves[i];

      wave.origin = glm::linearRand(glm::vec2(-50.0f, 50.0f), glm::vec2(-50.0f, 50.0f));
      wave.direction = glm::circularRand(1.0f);

      float phase = glm::linearRand(0.0f, static_cast<float>(2.0f * M_PI));
      wave.scale = { amplitude, wavelength, frequency, phase };

      wavelength *= 0.9f;
      amplitude *= 0.82f;
    }

    Vision::BufferDesc desc;
    desc.Size = m_NumWaves * sizeof(Wave);
    desc.Type = GL_UNIFORM_BUFFER;
    desc.Usage = GL_STATIC_DRAW;
    desc.Data = (void*)m_Waves;
    m_WaveBuffer = new Vision::Buffer(desc);
  }

  void GenerateMesh()
  {
    m_Mesh = Vision::MeshGenerator::CreatePlaneMesh(50.0f, 50.0f, 512, 512);
  }

  void GenerateShader()
  {
    m_WaterShader = new Vision::Shader("resources/waveShader.glsl");
    m_WaterShader->SetUniformBlock(m_WaveBuffer, "WaveProperties", 0); 
  }

private:
  Vision::PerspectiveCamera *m_PerspectiveCamera;
  Vision::Renderer *m_Renderer;
  Vision::ImGuiRenderer *m_UIRenderer;
  Vision::Mesh *m_Mesh;
  Vision::Shader *m_WaterShader;

  constexpr static std::size_t m_NumWaves = 100;
  Wave m_Waves[m_NumWaves];
  Vision::Buffer *m_WaveBuffer;
};

int main()
{
  Waves* waves = new Waves();
  waves->Run();
  delete waves;
}
