#pragma once

#include "renderer/RenderContext.h"

#include "renderer/opengl/GLDevice.h"

namespace Vision
{

class GLContext : public RenderContext
{
public:
  GLContext(SDL_Window* window);
  ~GLContext();

  RenderDevice* GetRenderDevice();
  PixelType GetPixelType() const { return PixelType::RGBA32; }
  void Present();

private:
  SDL_Window* window;
  SDL_GLContext context;
  GLDevice* device;
};

}
