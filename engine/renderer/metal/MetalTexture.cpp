#include "MetalTexture.h"

#include <stb_image.h>
#include <Metal/MTLTexture.hpp>

#include "MetalType.h"

namespace Vision
{

MetalTexture::MetalTexture(MTL::Device *device, float width, float height, PixelType pixelType)
{
  Resize(device, width, height);
}

MetalTexture::MetalTexture(MTL::Device *device, const char *filePath)
{
  int w, h, comp, desired;
  stbi_info(filePath, &w, &h, &comp);
  
  // desire four channels bc metal has no 24-bit type
  desired = (comp != 3) ? desired : 4;
  channels = desired;
  pixelType = ChannelsToPixelType(comp);

  // allocate our image
  Resize(device, static_cast<float>(w), static_cast<float>(h));

  // load from disc
  unsigned char *data = stbi_load(filePath, &w, &h, &channels, 4); 
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

}