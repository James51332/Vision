#pragma once

#include <vector>
#include <string>

namespace Vision
{

enum class PixelType
{
  R8,
  RG16,
  RGB24,
  RGBA32,
  Depth32,
  Depth24Stencil8,
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