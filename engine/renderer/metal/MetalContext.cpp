#include "MetalContext.h"

#include <SDL_metal.h>

namespace Vision
{

MetalContext::MetalContext(SDL_Window* wind)
  : window(wind)
{
  // First initialize the metal renderer
  view = SDL_Metal_CreateView(window);
  metalLayer = (CA::MetalLayer*)SDL_Metal_GetLayer(view);
  
  // Get the size of the window
  int w, h;
  SDL_GetWindowSizeInPixels(window, &w, &h);
  width = static_cast<float>(w);
  height = static_cast<float>(h);

  // Get the display scale.
  displayScale = SDL_GetWindowDisplayScale(window);

  // Then instantiate the GPU device
  MTL::Device* dev = MTL::CreateSystemDefaultDevice();
  metalLayer->setDevice(dev);
  device = new MetalDevice(dev, metalLayer, width, height);
}

MetalContext::~MetalContext()
{
  delete device;
  metalLayer->device()->release();

  SDL_Metal_DestroyView(view);
}

void MetalContext::Resize(float width, float height)
{
  // Resize in retina pixel space, which is typically doubled from the original.
  metalLayer->setDrawableSize(CGSizeMake(width * GetDisplayScale(), height * GetDisplayScale()));
  device->UpdateSize(width * GetDisplayScale(), height * GetDisplayScale());
}

}
