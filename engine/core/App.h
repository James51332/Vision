#pragma once

#include <SDL3/SDL.h>

#include "ui/UIInput.h"
#include "ui/ImGuiRenderer.h"

#include "renderer/RenderDevice.h"
#include "renderer/Renderer.h"
#include "renderer/Renderer2D.h"

namespace Vision
{

class App
{
public:
  App(const std::string& title = "Vision");
  ~App();

  void Run();
  void Stop() { m_Running = false; }

  static App* GetApp() { return appInstance; }
  static RenderDevice* GetDevice() { return appInstance->renderDevice; }

protected:
  virtual void OnUpdate(float timestep) = 0;
  virtual void OnResize(float width, float height) {} // Not mandatory to implement

private:
  void Init();
  void Shutdown();
  void ProcessEvents();

private:
  // A vision app is a singleton. One per process.
  static App* appInstance;

  // Run Loop
  bool m_Running = false;

  // Platform Data
  SDL_Window* m_Window;
  float m_DisplayScale = 1.0f; // Used for retina rendering
  std::string m_Title;
  
  // TODO: Other contexts.
  SDL_GLContext m_Context;

protected:
  float displayWidth = 1280.0f, displayHeight = 720.0f;

  RenderDevice* renderDevice;
  Renderer* renderer;
 // Renderer2D* renderer2D;
  ImGuiRenderer* uiRenderer;
};

}