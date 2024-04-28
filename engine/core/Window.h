#pragma once

#include <string>
#include <SDL.h>

#include "renderer/RenderContext.h"
#include "renderer/RenderAPI.h"

namespace Vision
{

struct WindowDesc
{
  float Width = 1280, Height = 720;
  std::string Title = "Vision";
  bool Resizeable = true;
  RenderAPI API = RenderAPI::OpenGL;
};

class Window
{
public:
  Window(const WindowDesc& desc);
  ~Window();

  void Show();
  void Hide();

  RenderContext* GetRenderContext() const { return context; }
  float GetDisplayScale() const { return displayScale; }

private:
  SDL_Window* window;
  float displayScale;
  RenderContext* context;
};

}