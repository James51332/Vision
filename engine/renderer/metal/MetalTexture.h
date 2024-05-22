#pragma once

#include <Metal/Metal.hpp>

#include "renderer/primitive/Texture.h"

namespace Vision
{

class MetalTexture
{
public:
  MetalTexture(MTL::Device* device, float width, float height, PixelType pixelType);
  MetalTexture(MTL::Device *device, const char *filePath);
  ~MetalTexture();

  void Resize(MTL::Device* device, float width, float height);
  void SetData(uint8_t *data);

  float GetWidth() const { return width; }
  float GetHeight() const { return height; }
  PixelType GetPixelType() const { return pixelType; }
  MTL::Texture* GetTexture() { return texture; }

private:
  MTL::Texture* texture = nullptr;

  float width, height;
  PixelType pixelType;
  int channels;
};

}