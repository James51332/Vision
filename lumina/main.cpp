#include "core/App.h"

#include <iostream>
#include <glm/gtc/random.hpp>

#include "renderer/shader/Shader.h"
#include "renderer/shader/ShaderCompiler.h"

namespace Lumina
{
  class Lumina : public Vision::App
  {
  public:
    Lumina()
      : Vision::App("Lumina")
    {
      // Create our compute pipeline
      Vision::ShaderCompiler compiler;
      Vision::ComputePipelineDesc desc;
      desc.ComputeKernels = { compiler.CompileFile("resources/computeShader.glsl") };

      Vision::ID pipeline = renderDevice->CreateComputePipeline(desc);

      // Create some data
      constexpr std::size_t elements = 50;
      constexpr std::size_t bufferSize = elements * sizeof(float);
      std::vector<float> data(elements);
      for (std::size_t i = 0; i < elements; i++)
      {
        data[i] = glm::linearRand(0.0f, 100.0f);
        std::cout << data[i] << " ";
      }
      std::cout << std::endl << std::endl << std::endl;

      // Copy the data to the GPU
      Vision::BufferDesc bufferDesc;
      bufferDesc.Type = Vision::BufferType::ShaderStorage;
      bufferDesc.Data = data.data();
      bufferDesc.Usage = Vision::BufferUsage::Dynamic;
      bufferDesc.Size = bufferSize;
      bufferDesc.DebugName = "Compute Buffer";
      Vision::ID computeBuffer = renderDevice->CreateBuffer(bufferDesc);

      // Tell the GPU to run our compute pass
      renderDevice->BeginCommandBuffer();
      renderDevice->BeginComputePass();

      renderDevice->SetComputeBuffer(computeBuffer);
      renderDevice->DispatchCompute(pipeline, "sum", {elements, 1, 1});
      renderDevice->DispatchCompute(pipeline, "triple", {elements, 1, 1});

      renderDevice->EndComputePass();
      renderDevice->SubmitCommandBuffer(true); // await completion

      // Fetch the data
      float* element;
      renderDevice->MapBufferData(computeBuffer, (void**)&element, bufferSize);

      // Log our new data
      if (element) // only print if the data exists.
      {
        for (std::size_t i = 0; i < elements; i++)
        {
          std::cout << element[i] << " ";
        }
        std::cout << std::endl;
      }

      renderDevice->FreeBufferData(computeBuffer, (void**)&element);

      // Prepare the renderer data
      Vision::RenderPassDesc rpDesc;
      rpDesc.ClearColor = { 0.2f, 0.2f, 0.2f, 1.0f };
      rpDesc.Framebuffer = 0;
      rpDesc.LoadOp = Vision::LoadOp::Clear;
      rpDesc.StoreOp = Vision::StoreOp::Store;
      renderPass = renderDevice->CreateRenderPass(rpDesc);

      camera = Vision::PerspectiveCamera(GetDisplayWidth(), GetDisplayHeight(), 0.1f, 50.0f);
      camera.SetPosition({0.0f,0.0f,2.0f});
    }

    void OnUpdate(float timestep)
    {
      camera.Update(timestep);

      if (!ShouldRender())
        return;

      renderDevice->BeginCommandBuffer();
      renderDevice->BeginRenderPass(renderPass);
      renderDevice->SetViewport(0, 0, GetDisplayWidth() * GetDisplayScale(), GetDisplayHeight() * GetDisplayScale());

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

    void OnResize(float width, float height)
    {
      camera.SetWindowSize(width, height);
    }

  private:
    Vision::ID renderPass;
    Vision::PerspectiveCamera camera;
  };
}

int main()
{
  Lumina::Lumina* lumina = new Lumina::Lumina();
  lumina->Run();
  delete lumina;
}
