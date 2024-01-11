#pragma once

#include <SDL3/SDL.h>

#include "renderer/Camera.h"
#include "renderer/Renderer.h"
#include "renderer/Mesh.h"
#include "renderer/Shader.h"

#include "ui/UIInput.h"
#include "ui/ImGuiRenderer.h"

namespace Vision
{

class App
{
public:
  App();
  ~App();

  void Run();
  void Stop() { m_Running = false; }

private:
  void GenerateWaves();
  void GenerateMesh();
  void GenerateShader();

private:
  void Init();
  void Shutdown();
  void ProcessEvents();

private:
  // Run Loop
  bool m_Running = false;

  // Platform Data
  SDL_Window* m_Window;
  SDL_GLContext m_Context;

  PerspectiveCamera* m_PerspectiveCamera;
  Renderer* m_Renderer;
  ImGuiRenderer* m_UIRenderer;
  Mesh* m_Mesh;
  Shader* m_WaterShader;

  struct Wave
  {
    glm::vec2 origin;
    glm::vec2 direction;
    glm::vec4 scale; // amplitude, period, frequency, dummy
  };

  Wave m_Waves[10];
  Buffer* m_WaveBuffer;
};

}