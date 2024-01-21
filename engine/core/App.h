#pragma once

#include <SDL3/SDL.h>

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

protected:
  virtual void OnUpdate(float timestep) = 0;
  virtual void OnResize() {} // Not mandatory to implement

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

protected:
  float m_DisplayWidth = 1280.0f, m_DisplayHeight = 720.0f;
  float m_DisplayScale = 1.0f; // Used for retina rendering
};

}