#include "core/App.h"

#include "renderer/Camera.h"
#include "renderer/Renderer.h"
#include "renderer/Mesh.h"
#include "renderer/MeshGenerator.h"
#include "renderer/Shader.h"
#include "renderer/Texture.h"

#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace Lumina
{
  class Lumina : public Vision::App
  {
  public:
    Lumina()
    {
      // Initialize the renderer
      m_Renderer = new Vision::Renderer(m_DisplayWidth, m_DisplayHeight, m_DisplayScale);
      m_PerspectiveCamera = new Vision::PerspectiveCamera(m_DisplayWidth, m_DisplayHeight, 0.1f, 100.0f);

      m_PlaneMesh = Vision::MeshGenerator::CreatePlaneMesh(5.0f, 5.0f, 100, 100);
      m_CubeMesh = Vision::MeshGenerator::CreateCubeMesh(1.0f);

      m_PhongShader = new Vision::Shader("resources/phongShader.glsl");
      m_TesselationShader = new Vision::Shader("resources/distShader.glsl");

      m_HeightMap = new Vision::Texture2D("resources/iceland_heightmap.png");
    }

    ~Lumina()
    {
      delete m_Renderer;
      delete m_PerspectiveCamera;

      delete m_PlaneMesh;
      delete m_CubeMesh;
      
      delete m_PhongShader;
      delete m_TesselationShader;

      delete m_HeightMap;
    }

    void OnUpdate(float timestep)
    {
      // Update Camera Controller
      m_PerspectiveCamera->Update(timestep);

      // Render
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, m_HeightMap->m_TextureID);

      float value = 16.0f;
      glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, &value);
      glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, &value);
      glPatchParameteri(GL_PATCH_VERTICES, 3);

      m_TesselationShader->Use();
      m_TesselationShader->UploadUniformInt(0, "heightMap");

      glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), {1.0f, 0.0f, 0.0f});

      m_Renderer->Begin(m_PerspectiveCamera);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      m_Renderer->DrawMesh(m_PlaneMesh, m_TesselationShader, rotation);
      m_Renderer->End();
    }

    void OnResize()
    {
      m_Renderer->Resize(m_DisplayWidth, m_DisplayHeight);
      m_PerspectiveCamera->SetWindowSize(m_DisplayWidth, m_DisplayHeight);
    }

  private:
    Vision::Renderer *m_Renderer;
    Vision::PerspectiveCamera *m_PerspectiveCamera;

    Vision::Mesh* m_PlaneMesh;
    Vision::Mesh* m_CubeMesh;

    Vision::Shader* m_PhongShader;
    Vision::Shader* m_TesselationShader;

    Vision::Texture2D* m_HeightMap;
};

}

int main()
{
  Lumina::Lumina *lumina = new Lumina::Lumina();
  lumina->Run();
  delete lumina;
}
