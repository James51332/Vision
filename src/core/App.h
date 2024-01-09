#pragma once

#include <SDL3/SDL.h>

#include "renderer/Camera.h"
#include "renderer/Renderer.h"
#include "renderer/Mesh.h"
#include "renderer/Shader.h"

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
  void Init();
  void Shutdown();
  void ProcessEvents();

private:
  // Run Loop
  bool m_Running = false;

  // Platform Data
  SDL_Window* m_Window;
  SDL_GLContext m_Context;

  OrthoCamera* m_Camera;
  PerspectiveCamera* m_PerspectiveCamera;
  Renderer* m_Renderer;
  Mesh* m_Mesh;
  Shader* m_WaterShader;
};

}