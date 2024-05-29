#include "MetalContext.h"

#include <SDL_metal.h>

namespace Vision
{

MetalContext::MetalContext(SDL_Window* wind)
  : window(wind)
{
  view = SDL_Metal_CreateView(window);
  metalLayer = (CA::MetalLayer*)SDL_Metal_GetLayer(view);

  MTL::Device* dev = MTL::CreateSystemDefaultDevice();
  metalLayer->setDevice(dev);
  device = new MetalDevice(dev, metalLayer);
}

MetalContext::~MetalContext()
{
  delete device;
  metalLayer->device()->release();

  SDL_Metal_DestroyView(view);
}

RenderDevice* MetalContext::GetRenderDevice()
{
  return device;
}

void MetalContext::Present()
{
//  metalLayer->nextDrawable()->present();
}

}
