#include "RenderDevice.h"

#include <SDL.h>

#include "renderer/opengl/GLDevice.h"

namespace Vision
{

RenderDevice* RenderDevice::Create(RenderAPI api)
{
  switch (api)
  {
    case RenderAPI::OpenGL: return new GLDevice();
    default:
      break;
  }

  SDL_assert(false);
  return nullptr;
}

}