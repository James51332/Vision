#include "core/App.h"

#include <glm/gtc/random.hpp>
#include <iostream>

#include "renderer/shader/Shader.h"
#include "renderer/shader/ShaderCompiler.h"

namespace Lumina
{
class Lumina : public Vision::App
{
public:
  Lumina() : Vision::App("Lumina")
  {
    Vision::ShaderCompiler compiler;
    std::vector<Vision::ShaderSPIRV> shaderSPIRV =
        compiler.CompileFile("resrouces/distShader.glsl");

    // Prepare the renderer data
    Vision::RenderPassDesc rpDesc;
    rpDesc.ClearColor = {0.2f, 0.2f, 0.2f, 1.0f};
    rpDesc.Framebuffer = 0;
    rpDesc.LoadOp = Vision::LoadOp::Clear;
    rpDesc.StoreOp = Vision::StoreOp::Store;
    renderPass = renderDevice->CreateRenderPass(rpDesc);

    camera = Vision::PerspectiveCamera(GetDisplayWidth(), GetDisplayHeight(), 0.1f, 50.0f);
    camera.SetPosition({0.0f, 0.0f, 2.0f});
  }

  void OnUpdate(float timestep)
  {
    camera.Update(timestep);

    if (!ShouldRender())
      return;

    renderDevice->BeginCommandBuffer();
    renderDevice->BeginRenderPass(renderPass);
    renderDevice->SetViewport(0, 0, GetDisplayWidth() * GetDisplayScale(),
                              GetDisplayHeight() * GetDisplayScale());

    renderer2D->Begin(&camera);
    renderer2D->DrawBox(glm::vec3(0.0f));
    renderer2D->End();

    uiRenderer->Begin();
    ImGui::ShowDemoWindow();
    uiRenderer->End();

    renderDevice->EndRenderPass();
    renderDevice->SchedulePresentation();
    renderDevice->SubmitCommandBuffer(false);
  }

  void OnResize(float width, float height) { camera.SetWindowSize(width, height); }

private:
  Vision::ID renderPass;
  Vision::PerspectiveCamera camera;

  Vision::ID distShader;
  Vision::Mesh* planeMesh;
};
} // namespace Lumina

int main()
{
  Lumina::Lumina* lumina = new Lumina::Lumina();
  lumina->Run();
  delete lumina;
}
