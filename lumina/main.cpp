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

      Vision::ShaderDesc shaderDesc;
      shaderDesc.FilePath = "resources/skyShader.glsl";
      skyboxShader = renderDevice->CreateShader(shaderDesc);

      Vision::PipelineDesc pipelineDesc;
      pipelineDesc.Layouts = 
      { Vision::BufferLayout({
        { Vision::ShaderDataType::Float3, "Normal" },
        { Vision::ShaderDataType::Float4, "Color" },
        { Vision::ShaderDataType::Float2, "UV" }}) 
      };
      pipelineDesc.Shader = skyboxShader;
      pipelineDesc.DepthFunc = Vision::DepthFunc::LessEqual;
      skyboxPS = renderDevice->CreatePipeline(pipelineDesc);

      Vision::RenderPassDesc rpDesc;
      rpDesc.Framebuffer = 0;
      rpDesc.ClearColor = {0.1f, 0.1f, 0.1f, 1.0f};
      rpDesc.LoadOp = Vision::LoadOp::Clear;
      renderPass = renderDevice->CreateRenderPass(rpDesc);
    }

    ~Lumina()
    {
      delete skyboxMesh;
      renderDevice->DestroyCubemap(skyboxTexture);
      renderDevice->DestroyShader(skyboxShader);
      renderDevice->DestroyPipeline(skyboxPS);

      renderDevice->DestroyRenderPass(renderPass);
    }

    void OnUpdate(float timestep)
    {
      // Update Camera Controller
      perspectiveCamera.Update(timestep);

      // Render
      // TODO: The renderer probably should own the render pass obj.
      renderDevice->BeginRenderPass(renderPass);
      renderer->Begin(&perspectiveCamera);

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

    Vision::Mesh* skyboxMesh;
    Vision::ID skyboxTexture;
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
