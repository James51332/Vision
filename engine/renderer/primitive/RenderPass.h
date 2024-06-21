#pragma once

#include <glm/glm.hpp>

#include "Framebuffer.h"

namespace Vision
{

enum class LoadOp
{
  Load,
  Clear,
  DontCare
};

enum class StoreOp
{
  Store,
  DontCare
};

struct RenderPassDesc
{
  ID Framebuffer = 0; // If this is set to zero, the screen buffer is the selected target.

  LoadOp LoadOp = LoadOp::Clear;
  glm::vec4 ClearColor;
  StoreOp StoreOp = StoreOp::Store;

  // TODO: Subpasses
};

using ID = std::size_t;

}