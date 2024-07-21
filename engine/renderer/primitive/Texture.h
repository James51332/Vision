#pragma once

#include <vector>
#include <string>

namespace Vision
{

enum class PixelType
{
  Invalid,
  
  // 8-bit per channel formats
  R8,
  RG8,
  RGBA8,
  BGRA8, // used by metal for framebuffers

  // 16-bit per channel formats
  R16,
  RG16,
  RGBA16,
  R16Float,
  RG16Float,
  RGBA16Float,

  // 32-bit per channel formats
  R32Uint,
  RG32Uint,
  RGBA32Uint,
  R32Float,
  RG32Float,
  RGBA32Float,

  // Depth formats
  Depth32Float,
  Depth24Stencil8
};

struct Texture2DDesc
{
  bool LoadFromFile = false;
  std::string FilePath;

  float Width;
  float Height;
  PixelType PixelType;
  bool WriteOnly = false;
  uint8_t* Data = nullptr;
};

// ----- Cubemaps -----

// right, left, top, bottom, front, back
struct CubemapDesc
{
  std::vector<std::string> Textures;
};

}