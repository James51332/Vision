#pragma once

#include "BufferLayout.h"

namespace Vision
{

enum class BufferType
{
  Vertex,
  Index,
  Uniform,
  ShaderStorage
};

// Buffer usage is very significant in Vision. Dynamic buffers are triple-backed so they can be
// modified even while the GPU is still using them. This creates trouble when reusing data for
// SSBOs. To resolve this, we explicitly note setting a buffer's data on the CPU will implicitly set
// the active buffer to the index of the current frame-in-flight. Since the user cannot access this,
// to access data written from the GPU or reuse it in a calculation, this creates the restriction
// that the data cannot be altered on the CPU afterwards.
enum class BufferUsage
{
  Static,
  Dynamic
};

struct BufferDesc
{
  BufferType Type;
  BufferUsage Usage;
  std::size_t Size;
  void* Data;

  std::string DebugName = "Unnamed";
};

using ID = std::size_t;

} // namespace Vision