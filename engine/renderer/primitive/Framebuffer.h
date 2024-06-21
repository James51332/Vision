#pragma once

#include <glad/glad.h>

#include "renderer/opengl/GLTexture.h"

#include "Texture.h"

namespace Vision
{

struct FramebufferDesc
{
  float Width, Height;
  PixelType ColorFormat;
  PixelType DepthType;
};

using ID = std::size_t;

}