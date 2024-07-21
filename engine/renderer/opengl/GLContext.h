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

  void Resize(float width, float height);

  RenderDevice* GetRenderDevice();
  float GetDisplayScale() const { return displayScale; }
  PixelType GetPixelType() const { return PixelType::RGBA8; }

private:
  SDL_Window* window;
  SDL_GLContext context;
  GLDevice* device;

  GLint versionMajor, versionMinor;

  float displayScale;
  float width, height;
};

}
