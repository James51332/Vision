#include "RenderContext.h"

#include "renderer/RenderAPI.h"

#include "renderer/opengl/GLContext.h"
#include "renderer/metal/MetalContext.h"

namespace Vision
{

RenderContext* RenderContext::Create(RenderAPI api, SDL_Window* window)
{
  switch (api)
  {
    case RenderAPI::OpenGL: return new GLContext(window);
    case RenderAPI::Metal: return new MetalContext(window);
    default:
      break;
  }

  SDL_assert(false);
  return nullptr;
}

}