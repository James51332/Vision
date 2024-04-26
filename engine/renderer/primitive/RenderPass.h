#pragma once

#include "Framebuffer.h"

namespace Vision
{

struct RenderPassDesc
{
  ID Framebuffer = 0; // If this is set to zero, the screen buffer is the selected target.

  // TODO: Subpasses
};

using ID = std::size_t;

}