#include "core/App.h"

#include "renderer/Camera.h"
#include "renderer/Renderer.h"
#include "renderer/Renderer2D.h"
#include "renderer/Mesh.h"
#include "renderer/MeshGenerator.h"
#include "renderer/Shader.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Lumina
{

class Lumina : public Vision::App
{
public:
  Lumina()
  {
    // Initialize the renderer
    m_Renderer = new Vision::Renderer(m_DisplayWidth, m_DisplayHeight, m_DisplayScale);
    m_Renderer2D = new Vision::Renderer2D(m_DisplayWidth, m_DisplayHeight, m_DisplayScale);
    m_PerspectiveCamera = new Vision::PerspectiveCamera(m_DisplayWidth, m_DisplayHeight);
    m_PerspectiveCamera->SetPosition({0.0f, 2.0f, 3.0f});

    GenerateMesh();
    GenerateShader();
  }

  ~Lumina()
  {
    delete m_Renderer;
    delete m_Renderer2D;
    delete m_PerspectiveCamera;

    delete m_Plane;
    delete m_Cube;

    delete m_PhongShader;
    delete m_GridShader;
  }

  void OnUpdate(float timestep)
  {
    // Update Camera Controller
    m_PerspectiveCamera->Update(timestep);

    // Render
    {
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      m_Renderer2D->Begin(m_PerspectiveCamera);

      m_Renderer2D->DrawQuad(glm::scale(glm::mat4(1.0f), glm::vec3(10.0f)), glm::vec4(0.0f, 1.0, 0.0f, 1.0f));
      m_Renderer2D->DrawCircle({5.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f }, 3.0f);
      m_Renderer2D->DrawLine({5.0f, 0.0f}, {0.0f, -5.0f});
      m_Renderer2D->DrawBox({0.0f, 0.0f}, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), 4.0f);

      m_Renderer2D->End();
    }
  }

  void OnResize()
  {
    m_Renderer->Resize(m_DisplayWidth, m_DisplayHeight);
    m_PerspectiveCamera->SetWindowSize(m_DisplayWidth, m_DisplayHeight);
  }

private:
  void GenerateMesh()
  {
    //m_Plane = Vision::MeshGenerator::CreatePlaneMesh(2.0f, 2.0f, 1, 1); // generate quad for plane
    m_Cube = Vision::MeshGenerator::CreateCubeMesh(1.0f);
  }

  void GenerateShader()
  {
    //m_PhongShader = new Vision::Shader("resources/phongShader.glsl");
    //m_GridShader = new Vision::Shader("resources/gridShader.glsl");
  }

private:
  Vision::Renderer *m_Renderer;
  Vision::Renderer2D *m_Renderer2D;
  Vision::PerspectiveCamera *m_PerspectiveCamera;

  Vision::Mesh *m_Plane;
  Vision::Mesh *m_Cube;

  Vision::Shader *m_GridShader;
  Vision::Shader *m_PhongShader;
};

}

int main()
{
  Lumina::Lumina *lumina = new Lumina::Lumina();
  lumina->Run();
  delete lumina;
}
