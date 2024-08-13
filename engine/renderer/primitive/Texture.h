#pragma once

#include <vector>
#include <string>
#include <iostream>

#include <SDL.h>

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

enum class MinMagFilter
{
  Nearest,
  Linear
};

struct Texture2DDesc
{
  bool LoadFromFile = false;
  std::string FilePath;

  float Width;
  float Height;
  PixelType PixelType;
  MinMagFilter MinFilter = MinMagFilter::Linear;
  MinMagFilter MagFilter = MinMagFilter::Linear;
  bool WriteOnly = false;
  uint8_t* Data = nullptr;
};

static uint32_t PixelTypeBytesPerPixel(PixelType type)
{
  switch (type)
  {
    case PixelType::R8: return 1;
    case PixelType::RG8: return 2;
    case PixelType::RGBA8: return 4;
    case PixelType::BGRA8: return 4;
    case PixelType::R16: return 2;
    case PixelType::RG16: return 4;
    case PixelType::RGBA16: return 8;
    case PixelType::R16Float: return 2;
    case PixelType::RG16Float: return 4;
    case PixelType::RGBA16Float: return 8;
    case PixelType::R32Uint: return 4;
    case PixelType::RG32Uint: return 8;
    case PixelType::RGBA32Uint: return 16;
    case PixelType::R32Float: return 4;
    case PixelType::RG32Float: return 8;
    case PixelType::RGBA32Float: return 16;
    case PixelType::Depth32Float: return 4;
    case PixelType::Depth24Stencil8: return 4;
    default: break;
  }

  std::cout << "Invalid PixelType!" << std::endl;
  SDL_assert(false);
  return 0;
}

// ----- Cubemaps -----

// right, left, top, bottom, front, back
struct CubemapDesc
{
  std::vector<std::string> Textures;
};

}