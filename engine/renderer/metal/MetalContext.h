#pragma once

#include "renderer/RenderContext.h"

#include <Metal/Metal.hpp>
#include <QuartzCore/CAMetalLayer.hpp>
#include <SDL.h>

namespace Vision
{

class MetalContext : public RenderContext
{
public:
  MetalContext(SDL_Window* window);
  ~MetalContext();

  RenderDevice* GetRenderDevice();
  void Present();

private:
  SDL_Window* window;
  SDL_MetalView view;
  CA::MetalLayer* metalLayer;
};

}