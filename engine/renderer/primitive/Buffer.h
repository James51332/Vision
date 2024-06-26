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

}