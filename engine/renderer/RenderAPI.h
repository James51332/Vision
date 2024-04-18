#pragma once

namespace Vision
{

// ----- RenderAPI -----

// A render API represents the underlying graphics API that is used for the renderer.
// For now, we only have support for OpenGL. I'd like to add metal next, so that we can get
// support for compute shaders on macOS. We already have a shader cross compiler built into
// this project, so we should be able to accept any type of shader, and build it.

enum class RenderAPI
{
  OpenGL,
  Metal,
  Vulkan,
  DX12,
};

}