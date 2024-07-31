#pragma once

#include <SDL3/SDL.h>

#include "core/Window.h"

#include "ui/UIInput.h"
#include "ui/ImGuiRenderer.h"

#include "renderer/RenderDevice.h"
#include "renderer/RenderContext.h"
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
  void Stop() { running = false; }

  static App* GetApp() { return appInstance; }
  static RenderDevice* GetDevice() { return appInstance->renderDevice; }
  static PixelType GetPixelFormat() { return appInstance->renderContext->GetPixelType(); }

protected:
  virtual void OnUpdate(float timestep) = 0;
  virtual void OnResize(float width, float height) {} // Not mandatory to implement

  float GetDisplayWidth() const { return displayWidth; }
  float GetDisplayHeight() const { return displayHeight; }
  float GetDisplayScale() const { return displayScale; }
  bool ShouldRender() const { return !displayOccluded; }

private:
  void Init();
  void Shutdown();
  void ProcessEvents();

private:
  // A vision app is a singleton. One per process.
  static App* appInstance;

  // Run Loop
  bool running = true;

  // Platform Data
  Window* window;
  std::string title;
  float displayWidth = 1280.0f, displayHeight = 720.0f;
  float displayScale; // Used for retina rendering
  bool displayOccluded = false;

protected:
  RenderDevice* renderDevice;
  RenderContext* renderContext;
  Renderer* renderer;
  Renderer2D* renderer2D;
  ImGuiRenderer* uiRenderer;
};

}
