#include "Window.h"

namespace Vision
{

Window::Window(const WindowDesc& desc)
{
  unsigned int flags = SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN;
  if (desc.Resizeable) flags |= SDL_WINDOW_RESIZABLE;

  // Setup our various API settings
  if (desc.API == RenderAPI::OpenGL) flags |= SDL_WINDOW_OPENGL;
  else if (desc.API == RenderAPI::Metal) flags |= SDL_WINDOW_METAL;

  window = SDL_CreateWindow(desc.Title.c_str(), desc.Width, desc.Height, flags);
  context = RenderContext::Create(desc.API, window);
}

Window::~Window()
{
  delete context;
  SDL_DestroyWindow(window);
}

void Window::Show()
{
  SDL_ShowWindow(window);
}

void Window::Hide()
{
  SDL_HideWindow(window);
}

}