#pragma once

#include <SDL.h>

#include "renderer/RenderDevice.h"
#include "renderer/RenderAPI.h"

namespace Vision
{

class RenderContext
{
public:
  static RenderContext* Create(RenderAPI api, SDL_Window* window);
  virtual ~RenderContext() = default;

  virtual RenderDevice* GetRenderDevice() = 0;
  virtual void Present() = 0;
};

}
