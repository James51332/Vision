#include "MetalContext.h"

#include <SDL_metal.h>

namespace Vision
{

MetalContext::MetalContext(SDL_Window* wind)
  : window(wind)
{
  view = SDL_Metal_CreateView(window);
  metalLayer = (CA::MetalLayer*)SDL_Metal_GetLayer(view);
  metalLayer->setDevice(MTL::CreateSystemDefaultDevice());
}

MetalContext::~MetalContext()
{
  SDL_Metal_DestroyView(view);
}

RenderDevice* MetalContext::GetRenderDevice()
{
  return nullptr;
}

void MetalContext::Present()
{
  metalLayer->nextDrawable()->present();
}

}