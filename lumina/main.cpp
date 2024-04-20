#include "core/App.h"

#include "core/Input.h"

#include "renderer/Camera.h"
#include "renderer/Renderer.h"
#include "renderer/Mesh.h"
#include "renderer/MeshGenerator.h"
#include "renderer/primitive/Shader.h"
#include "renderer/primitive/Texture.h"

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
      Vision::ShaderDesc sd1;
      sd1.filePath = "resources/distShader.glsl";
      tesselationShader = Vision::RenderDevice::CreateShader(sd1);

      Vision::PipelineDesc p1;
      p1.Layouts = { planeMesh->m_VertexBuffer->GetLayout() };
      p1.Shader = tesselationShader;
      tesselationPS = Vision::RenderDevice::CreatePipeline(p1);

      // We're cooked until we figure out how to do this without breaking everything.
      // tesselationShader->Use();
      // tesselationShader->UploadUniformInt(0, "heightMap");

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
      skyboxTexture = new Vision::Cubemap(desc);

      skyboxMesh = Vision::MeshGenerator::CreateCubeMesh(1.0f);

      Vision::ShaderDesc sd;
      sd.filePath = "resources/skyShader.glsl";
      skyboxShader = Vision::RenderDevice::CreateShader(sd);
      // skyboxShader->Use();
      // skyboxShader->UploadUniformInt(0, "skybox");

      Vision::PipelineDesc p2;
      p2.Layouts = { skyboxMesh->m_VertexBuffer->GetLayout() };
      p2.Shader = skyboxShader;
      skyboxPS = Vision::RenderDevice::CreatePipeline(p2);
    }

    ~Lumina()
    {
      delete planeMesh;
      Vision::RenderDevice::DestroyShader(tesselationShader);
      delete heightMap;

      delete skyboxMesh;
      delete skyboxTexture;
      Vision::RenderDevice::DestroyShader(skyboxShader);
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
      renderer->DrawMesh(planeMesh, tesselationPS);

      // Skybox
      // glDepthFunc(GL_LEQUAL);
      // skyboxTexture->Bind();      
      // renderer->DrawMesh(skyboxMesh, skyboxPS);

      renderer->End();
    }

    void OnResize(float width, float height)
    {
      perspectiveCamera.SetWindowSize(width, height);
    }

  private:
    Vision::PerspectiveCamera perspectiveCamera;

    Vision::Mesh* planeMesh;
    Vision::ID tesselationShader;
    Vision::ID tesselationPS;
    Vision::Texture2D* heightMap;

    Vision::Cubemap* skyboxTexture;
    Vision::Mesh* skyboxMesh;
    Vision::ID skyboxPS;
    Vision::ID skyboxShader;
};

}

int main()
{
  Lumina::Lumina *lumina = new Lumina::Lumina();
  lumina->Run();
  delete lumina;
}
