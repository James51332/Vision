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

  virtual void Resize(float width, float height) = 0;

  virtual RenderDevice* GetRenderDevice() = 0;
  virtual float GetDisplayScale() const = 0;
  virtual PixelType GetPixelType() const = 0;
};

}
