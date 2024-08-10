#include "MetalTexture.h"

#include <stb_image.h>
#include <SDL.h>
#include <Metal/MTLTexture.hpp>

#include "MetalType.h"

namespace Vision
{

static MTL::SamplerState* NewSamplerState(MTL::Device* device)
{
  MTL::SamplerDescriptor* samplerDesc = MTL::SamplerDescriptor::alloc()->init();
  samplerDesc->setMinFilter(MTL::SamplerMinMagFilterLinear);
  samplerDesc->setMagFilter(MTL::SamplerMinMagFilterNearest);
  MTL::SamplerState* samplerState = device->newSamplerState(samplerDesc);
  samplerDesc->release();
  return samplerState;
}

MetalTexture::MetalTexture(MTL::Device *device, float width, float height, PixelType pixel)
	: samplerState(NewSamplerState(device)), pixelType(pixel), channels(PixelTypeToChannels(pixel))
{
  Resize(device, width, height);
}

MetalTexture::MetalTexture(MTL::Device *device, const char *filePath)
	: samplerState(NewSamplerState(device))
{
  int w, h, comp;
  stbi_info(filePath, &w, &h, &comp);
  
  // desire four channels bc metal has no 24-bit type
  channels = (comp != 3) ? comp : 4;
  pixelType = ChannelsToPixelType(channels);

  // allocate our image
  Resize(device, static_cast<float>(w), static_cast<float>(h));

  // load from disc
  unsigned char *data = stbi_load(filePath, &w, &h, nullptr, channels);
  if (!data)
  {
    std::cout << "Failed to load image:" << std::endl;
    std::cout << stbi_failure_reason() << std::endl;
  }

  // set the data
  SetData(data);
  stbi_image_free(data);
}

MetalTexture::~MetalTexture()
{
  texture->release();
  samplerState->release();
}

void MetalTexture::Resize(MTL::Device* device, float w, float h)
{
  if (texture) 
  {
    texture->release();
    texture = nullptr;
  }

  width = w;
  height = h;

  MTL::TextureDescriptor* texDesc = MTL::TextureDescriptor::alloc()->init();
  texDesc->setWidth(width);
  texDesc->setHeight(height);
  texDesc->setPixelFormat(PixelTypeToMTLPixelFormat(pixelType));
  texDesc->setUsage(MTL::TextureUsageUnknown); // TODO: This is prob important for perf.

  texture = device->newTexture(texDesc);
}

void MetalTexture::SetData(uint8_t *data)
{
  MTL::Region region(0, 0, width, height);
  texture->replaceRegion(region, 0, data, width * channels);
}

void MetalTexture::SetDataRaw(void *data)
{
  MTL::Region region(0, 0, width, height);
  texture->replaceRegion(region, 0, data, width * PixelTypeBytesPerPixel(pixelType));
}

MetalCubemap::MetalCubemap(MTL::Device* device, const CubemapDesc& desc)
{
  SDL_assert(desc.Textures.size() == 6);

  // desire four channels bc metal has no 24-bit type
  int w, h, comp;
  stbi_info(desc.Textures[0].c_str(), &w, &h, &comp);

  int channels = (comp != 3) ? comp : 4;
  pixelType = ChannelsToPixelType(channels);

  MTL::TextureDescriptor* descriptor;
  descriptor = MTL::TextureDescriptor::alloc()->textureCubeDescriptor(PixelTypeToMTLPixelFormat(pixelType),
                                                                      w,
                                                                      false);
  
  cubemap = device->newTexture(descriptor);

  // attach the sides to each
  int side = 0;
  for (auto file : desc.Textures)
  {
    unsigned char *data = stbi_load(file.c_str(), &w, &h, nullptr, channels);
    if (!data)
    {
      std::cout << "Failed to load image:" << file << std::endl;
      std::cout << stbi_failure_reason() << std::endl;
    }

    MTL::Region region(0, 0, w, h);
    cubemap->replaceRegion(region, 0, side, data, w * channels, w * h * channels);

    side++;
    stbi_image_free(data);
  }

  MTL::SamplerDescriptor *samplerDesc = MTL::SamplerDescriptor::alloc()->init();
  samplerDesc->setMinFilter(MTL::SamplerMinMagFilterLinear);
  samplerDesc->setMagFilter(MTL::SamplerMinMagFilterLinear);
  samplerState = device->newSamplerState(samplerDesc);
  samplerDesc->release();
}

MetalCubemap::~MetalCubemap()
{
  cubemap->release();
  samplerState->release();
}

}
