#include "core/App.h"

#include "renderer/Camera.h"
#include "renderer/Renderer.h"
#include "renderer/Renderer2D.h"
#include "renderer/Mesh.h"
#include "renderer/MeshGenerator.h"
#include "renderer/Shader.h"
#include "renderer/Framebuffer.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Lumina
{

class Lumina : public Vision::App
{
public:
  Lumina()
  {
    // Initialize the renderer
    m_Renderer2D = new Vision::Renderer2D(m_DisplayWidth, m_DisplayHeight, m_DisplayScale);
    m_PerspectiveCamera = new Vision::PerspectiveCamera(m_DisplayWidth, m_DisplayHeight);
    m_PerspectiveCamera->SetPosition({0.0f, 5.0f, 0.0f});
    m_PerspectiveCamera->SetRotation({-90.0f, 0.0f, 0.0f});
  }

  ~Lumina()
  {
    delete m_Renderer2D;
    delete m_PerspectiveCamera;
  }

  void OnUpdate(float timestep)
  {
    // Update Camera Controller
    m_PerspectiveCamera->Update(timestep);

    // Render
    static float radius = 0.0f;
    {
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      m_Renderer2D->Begin(m_PerspectiveCamera, true, glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), { 1.0f, 0.0f, 0.0f }));
      {
        for (float x = -10.0f; x <= 10.0f; x++)
        {
          for (float y = -10.0f; y <= 10.0f; y++)
          {
            glm::vec4 color(1.0f);
            if (glm::length(glm::vec2(x, y)) >= radius)
            {
              color = glm::vec4((x + 10.0f) / 20.0f, (y + 10.0f) / 20.0f, 0.0f, 1.0f);
            }
            m_Renderer2D->DrawPoint({x, y}, color, 0.25f);
          }
        }

        m_Renderer2D->DrawCircle({0.0f, 0.0f}, glm::vec4(1.0f), radius, 0.25f);
      }
      m_Renderer2D->End();
    }

    radius += timestep;
  }

  void OnResize()
  {
    m_Renderer2D->Resize(m_DisplayWidth, m_DisplayHeight);
    m_PerspectiveCamera->SetWindowSize(m_DisplayWidth, m_DisplayHeight);
  }

private:
  Vision::Renderer2D *m_Renderer2D;
  Vision::PerspectiveCamera *m_PerspectiveCamera;
};

}

int main()
{
  Lumina::Lumina *lumina = new Lumina::Lumina();
  lumina->Run();
  delete lumina;
}
