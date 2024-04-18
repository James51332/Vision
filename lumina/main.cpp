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
      : Vision::App("Lumina")
    {
      // Initialize the scene
      perspectiveCamera = Vision::PerspectiveCamera(displayWidth, displayHeight, 0.1f, 1000.0f);

      planeMesh = Vision::MeshGenerator::CreatePlaneMesh(50.0f, 50.0f, 128, 128, true, false);
      tesselationShader = new Vision::Shader("resources/distShader.glsl");
      tesselationShader->Use();
      tesselationShader->UploadUniformInt(0, "heightMap");

      heightMap = new Vision::Texture2D("resources/iceland_heightmap.png");

      Vision::CubemapDesc desc;
      desc.Textures = {
        "resources/skybox/right.jpg",
        "resources/skybox/left.jpg",
        "resources/skybox/top.jpg",
        "resources/skybox/bottom.jpg",
        "resources/skybox/front.jpg",
        "resources/skybox/back.jpg"
      };
      skyboxMesh = Vision::MeshGenerator::CreateCubeMesh(1.0f);
      skyboxTexture = new Vision::Cubemap(desc);
      skyboxShader = new Vision::Shader("resources/skyShader.glsl");
      skyboxShader->Use();
      skyboxShader->UploadUniformInt(0, "skybox");
    }

    ~Lumina()
    {
      delete planeMesh;
      delete tesselationShader;
      delete heightMap;

      delete skyboxMesh;
      delete skyboxTexture;
      delete skyboxShader;
    }

    void OnUpdate(float timestep)
    {
      // Update Camera Controller
      perspectiveCamera.Update(timestep);

      // Render
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // for debugging
      if (Vision::Input::KeyDown(SDL_SCANCODE_TAB))
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      
      renderer->Begin(&perspectiveCamera);

      heightMap->Bind();
      renderer->DrawMesh(planeMesh, tesselationShader);

      // Skybox
      glDepthFunc(GL_LEQUAL);
      skyboxTexture->Bind();      
      renderer->DrawMesh(skyboxMesh, skyboxShader);

      renderer->End();
    }

    void OnResize(float width, float height)
    {
      perspectiveCamera.SetWindowSize(width, height);
    }

  private:
    Vision::PerspectiveCamera perspectiveCamera;

    Vision::Mesh* planeMesh;
    Vision::Shader* tesselationShader;
    Vision::Texture2D* heightMap;

    Vision::Cubemap* skyboxTexture;
    Vision::Mesh* skyboxMesh;
    Vision::Shader* skyboxShader;
};

}

int main()
{
  Lumina::Lumina *lumina = new Lumina::Lumina();
  lumina->Run();
  delete lumina;
}
