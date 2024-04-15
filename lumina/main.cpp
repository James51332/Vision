#include "core/App.h"

#include "core/Input.h"

#include "renderer/Camera.h"
#include "renderer/Renderer.h"
#include "renderer/Mesh.h"
#include "renderer/MeshGenerator.h"
#include "renderer/Shader.h"
#include "renderer/Texture.h"

namespace Lumina
{
  class Lumina : public Vision::App
  {
  public:
    Lumina()
    {
      // Initialize the renderer
      m_Renderer = new Vision::Renderer(m_DisplayWidth, m_DisplayHeight, m_DisplayScale);
      m_PerspectiveCamera = new Vision::PerspectiveCamera(m_DisplayWidth, m_DisplayHeight, 0.1f, 1000.0f);

      m_PlaneMesh = Vision::MeshGenerator::CreatePlaneMesh(50.0f, 50.0f, 128, 128, true, false);
      m_TesselationShader = new Vision::Shader("resources/distShader.glsl");
      m_TesselationShader->Use();
      m_TesselationShader->UploadUniformInt(0, "heightMap");

      m_HeightMap = new Vision::Texture2D("resources/iceland_heightmap.png");

      Vision::CubemapDesc desc;
      desc.Textures = {
        "resources/skybox/right.jpg",
        "resources/skybox/left.jpg",
        "resources/skybox/top.jpg",
        "resources/skybox/bottom.jpg",
        "resources/skybox/front.jpg",
        "resources/skybox/back.jpg"
      };
      m_SkyMesh = Vision::MeshGenerator::CreateCubeMesh(1.0f);
      m_Skybox = new Vision::Cubemap(desc);
      m_SkyShader = new Vision::Shader("resources/skyShader.glsl");
      m_SkyShader->Use();
      m_SkyShader->UploadUniformInt(0, "skybox");
    }

    ~Lumina()
    {
      delete m_Renderer;
      delete m_PerspectiveCamera;

      delete m_PlaneMesh;
      delete m_TesselationShader;
      delete m_HeightMap;

      delete m_SkyMesh;
      delete m_Skybox;
      delete m_SkyShader;
    }

    void OnUpdate(float timestep)
    {
      // Update Camera Controller
      m_PerspectiveCamera->Update(timestep);

      // Render
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // for debugging
      if (Vision::Input::KeyDown(SDL_SCANCODE_TAB))
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      
      m_Renderer->Begin(m_PerspectiveCamera);

      m_HeightMap->Bind();
      m_Renderer->DrawMesh(m_PlaneMesh, m_TesselationShader);

      // Skybox
      glDepthFunc(GL_LEQUAL);
      m_Skybox->Bind();      
      m_Renderer->DrawMesh(m_SkyMesh, m_SkyShader);

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
    Vision::Shader* m_TesselationShader;
    Vision::Texture2D* m_HeightMap;

    Vision::Cubemap* m_Skybox;
    Vision::Mesh* m_SkyMesh;
    Vision::Shader* m_SkyShader;
};

}

int main()
{
  Lumina::Lumina *lumina = new Lumina::Lumina();
  lumina->Run();
  delete lumina;
}
