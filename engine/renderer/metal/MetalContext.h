#pragma once

#include <Metal/Metal.hpp>
#include <QuartzCore/CAMetalLayer.hpp>
#include <SDL.h>

#include "renderer/RenderContext.h"
#include "renderer/metal/MetalDevice.h"

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
  SDL_Window* window = nullptr;
  SDL_MetalView view;
  CA::MetalLayer* metalLayer = nullptr;

  MetalDevice* device = nullptr;
};

}