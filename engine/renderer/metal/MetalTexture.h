#pragma once

#include <Metal/Metal.hpp>

#include "renderer/primitive/Texture.h"

namespace Vision
{

class MetalTexture
{
public:
  MetalTexture(MTL::Device* device, float width, float height, PixelType pixelType, MinMagFilter minFilter, MinMagFilter magFilter);
  MetalTexture(MTL::Device *device, const char *filePath, MinMagFilter minFilter, MinMagFilter magFilter);
  ~MetalTexture();

  void Resize(MTL::Device* device, float width, float height);
  void SetData(uint8_t *data);
  void SetDataRaw(void *data);

  float GetWidth() const { return width; }
  float GetHeight() const { return height; }
  PixelType GetPixelType() const { return pixelType; }
  MTL::Texture* GetTexture() { return texture; }
  MTL::SamplerState* GetSampler() const { return samplerState; }

private:
  MTL::Texture* texture = nullptr;
  MTL::SamplerState* samplerState = nullptr;

  float width, height;
  PixelType pixelType;
  int channels;
};

class MetalCubemap
{
public:
  MetalCubemap(MTL::Device* device, const CubemapDesc &desc);
  ~MetalCubemap();

  MTL::Texture* GetTexture() const { return cubemap; }
  MTL::SamplerState* GetSampler() const { return samplerState; }

private:
  MTL::Texture* cubemap;
  MTL::SamplerState *samplerState = nullptr;

  PixelType pixelType;
};
}