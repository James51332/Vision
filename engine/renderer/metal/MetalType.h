#pragma once

#include <Metal/Metal.hpp>
#include <Metal/MTLPixelFormat.hpp>

#include <iostream>

#include "renderer/primitive/Texture.h"

namespace Vision
{

static MTL::PixelFormat PixelTypeToMTLPixelFormat(PixelType type)
{
  switch (type)
  {
    case PixelType::R8: return MTL::PixelFormatR8Unorm;
    case PixelType::RG16: return MTL::PixelFormatRG16Unorm;
    //case PixelType::RGB24: return MTL::PixelFormatRGB24Snorm;
    case PixelType::RGBA32: return MTL::PixelFormatRGBA8Unorm;
    case PixelType::BGRA32: return MTL::PixelFormatBGRA8Unorm;
    case PixelType::Depth32: return MTL::PixelFormatDepth32Float;
    case PixelType::Depth24Stencil8: return MTL::PixelFormatDepth24Unorm_Stencil8;
    default:
      break;
  }

  std::cout << "PixelType Unsupported in Metal!" << std::endl;
  return MTL::PixelFormatInvalid;
}

static PixelType MTLPixelFormatToPixelType(MTL::PixelFormat type)
{
  switch (type)
  {
    case MTL::PixelFormatR8Unorm: return PixelType::R8;
    case MTL::PixelFormatRG16Unorm: return PixelType::RG16;
    case MTL::PixelFormatRGBA8Unorm: return PixelType::RGBA32;
    case MTL::PixelFormatBGRA8Unorm: return PixelType::BGRA32;
    case MTL::PixelFormatDepth32Float: return PixelType::Depth32;
    case MTL::PixelFormatDepth24Unorm_Stencil8: return PixelType::Depth24Stencil8;
    default:
      break;
  }

  std::cout << "Unknown MTLPixelType!" << std::endl;
  return PixelType::R8;
}

static MTL::VertexFormat ShaderDataTypeToMTLVertexFormat(ShaderDataType type)
{
  switch (type)
  {
    case ShaderDataType::Int: return MTL::VertexFormatInt;
    case ShaderDataType::Float: return MTL::VertexFormatFloat;
    case ShaderDataType::Float2: return MTL::VertexFormatFloat2;
    case ShaderDataType::Float3: return MTL::VertexFormatFloat3;
    case ShaderDataType::Float4: return MTL::VertexFormatFloat4;
    case ShaderDataType::UByte4: return MTL::VertexFormatUChar;
    default:
      break;
  }

  std::cout << "Unknown Shader Data Type!" << std::endl;
  return MTL::VertexFormatInvalid;
}

}