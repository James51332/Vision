#include "RenderContext.h"

#include "core/Macros.h"

#include "renderer/RenderAPI.h"

#include "renderer/opengl/GLContext.h"
#include "renderer/metal/MetalContext.h"

namespace Vision
{

RenderContext* RenderContext::Create(RenderAPI api, SDL_Window* window)
{
#ifdef VISION_MACOS
  switch (api)
  {
    case RenderAPI::OpenGL: return new GLContext(window);
    case RenderAPI::Metal: return new MetalContext(window);
    default:
      break;
  }

  std::cout << "Unsupported RenderAPI on macOS!" << std::endl;
  SDL_assert(false);
  return nullptr;
#endif

#ifdef VISION_WINDOWS
  switch (api)
  {
    case RenderAPI::OpenGL: return new GLContext(window);
    default:
      break;
  }

  std::cout << "Unsupported RenderAPI on Windows!" << std::endl;
  SDL_assert(false);
  return nullptr;
#endif
}

}