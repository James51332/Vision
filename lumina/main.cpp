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
      sd1.FilePath = "resources/distShader.glsl";
      tesselationShader = renderDevice->CreateShader(sd1);

      Vision::PipelineDesc p1;
      p1.Layouts = { Vision::BufferLayout({{Vision::ShaderDataType::Float3, "Position"},
                     {Vision::ShaderDataType::Float3, "Normal"},
                     {Vision::ShaderDataType::Float4, "Color"},
                     {Vision::ShaderDataType::Float2, "UV"}}) };
      p1.Shader = tesselationShader;
      tesselationPS = renderDevice->CreatePipeline(p1);

      Vision::Texture2DDesc td;
      td.LoadFromFile = true;
      td.FilePath = "resources/iceland_heightmap.png";
      heightMap = renderDevice->CreateTexture2D(td);

      Vision::CubemapDesc desc;
      desc.Textures = {
        "resources/skybox/right.jpg",
        "resources/skybox/left.jpg",
        "resources/skybox/top.jpg",
        "resources/skybox/bottom.jpg",
        "resources/skybox/front.jpg",
        "resources/skybox/back.jpg"
      };
      skyboxTexture = renderDevice->CreateCubemap(desc);

      skyboxMesh = Vision::MeshGenerator::CreateCubeMesh(1.0f);

      Vision::ShaderDesc sd;
      sd.FilePath = "resources/skyShader.glsl";
      skyboxShader = renderDevice->CreateShader(sd);

      Vision::PipelineDesc p2;
      p2.Layouts = p1.Layouts;
      p2.Shader = skyboxShader;
      p2.DepthFunc = Vision::DepthFunc::LessEqual;
      skyboxPS = renderDevice->CreatePipeline(p2);

      Vision::RenderPassDesc rpDesc;
      rpDesc.Framebuffer = 0;
      rpDesc.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
      rpDesc.LoadOp = Vision::LoadOp::Clear;
      renderPass = renderDevice->CreateRenderPass(rpDesc);
    }

    ~Lumina()
    {
      delete planeMesh;
      renderDevice->DestroyShader(tesselationShader);
      renderDevice->DestroyTexture2D(heightMap);

      renderDevice->DestroyCubemap(skyboxTexture);
      delete skyboxMesh;
      renderDevice->DestroyShader(skyboxShader);

      renderDevice->DestroyRenderPass(renderPass);
    }

    void OnUpdate(float timestep)
    {
      // Update Camera Controller
      perspectiveCamera.Update(timestep);

      // Render
      renderDevice->BeginRenderPass(renderPass);
      renderer->Begin(&perspectiveCamera);

      renderDevice->BindTexture2D(heightMap);
      renderer->DrawMesh(planeMesh, tesselationPS);
      renderDevice->BindCubemap(skyboxTexture);
      renderer->DrawMesh(skyboxMesh, skyboxPS);

      renderer->End();
      renderDevice->EndRenderPass();
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
    Vision::ID heightMap;

    Vision::ID skyboxTexture;
    Vision::Mesh* skyboxMesh;
    Vision::ID skyboxPS;
    Vision::ID skyboxShader;

    Vision::ID renderPass;
};

}

int main()
{
  Lumina::Lumina *lumina = new Lumina::Lumina();
  lumina->Run();
  delete lumina;
}
