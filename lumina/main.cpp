#include "core/App.h"

#include <iostream>
#include <glm/gtc/random.hpp>

namespace Lumina
{
  class Lumina : public Vision::App
  {
  public:
    Lumina()
        : Vision::App("Lumina")
    {
      /*// Create our compute pipeline
      Vision::ComputePipelineDesc desc;
      desc.FilePath = "resources/computeShader.glsl";
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
      std::cout << std::endl;

      // Copy the data to the GPU
      Vision::BufferDesc bufferDesc;
      bufferDesc.Data = data.data();
      bufferDesc.Usage = Vision::BufferUsage::Dynamic;
      bufferDesc.Size = bufferSize;
      bufferDesc.DebugName = "Compute Buffer";
      Vision::ID computeBuffer = renderDevice->CreateBuffer(bufferDesc);

      // Tell the GPU to run our compute pass
      renderDevice->BeginCommandBuffer();
      renderDevice->BeginComputePass();

      renderDevice->SetComputeBuffer(computeBuffer);
      renderDevice->DispatchCompute(pipeline, {elements, 1, 1});

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

      // Close the app
      Stop();*/
    }

    void OnUpdate(float timestep)
    {
      uiRenderer->Begin();
      ImGui::ShowDemoWindow();
      uiRenderer->End();

//      renderDevice->BeginCommandBuffer();
//      renderDevice->SchedulePresentation();
//      renderDevice->SubmitCommandBuffer();
    }
  };
}

int main()
{
  Lumina::Lumina *lumina = new Lumina::Lumina();
  lumina->Run();
  delete lumina;
}
