#pragma once

#include <Metal/Metal.hpp>
#include <Metal/MTLPixelFormat.hpp>

#include <iostream>

#include "renderer/primitive/Texture.h"
#include "renderer/primitive/Buffer.h"
#include "renderer/primitive/Pipeline.h"
#include "renderer/primitive/RenderPass.h"

#include "renderer/RenderCommand.h"

namespace Vision
{

static MTL::PixelFormat PixelTypeToMTLPixelFormat(PixelType type)
{
  switch (type)
  {
    case PixelType::R8: return MTL::PixelFormatR8Unorm;
    case PixelType::RG8: return MTL::PixelFormatRG8Unorm;
    case PixelType::RGBA8: return MTL::PixelFormatRGBA8Unorm;
    case PixelType::BGRA8: return MTL::PixelFormatBGRA8Unorm;
    case PixelType::R16: return MTL::PixelFormatR16Unorm;
    case PixelType::RG16: return MTL::PixelFormatRG16Unorm;
    case PixelType::RGBA16: return MTL::PixelFormatRGBA16Unorm;
    case PixelType::R16Float: return MTL::PixelFormatR16Float;
    case PixelType::RG16Float: return MTL::PixelFormatRG16Float;
    case PixelType::RGBA16Float: return MTL::PixelFormatRGBA16Float;
    case PixelType::R32Uint: return MTL::PixelFormatR32Uint;
    case PixelType::RG32Uint: return MTL::PixelFormatRG32Uint;
    case PixelType::RGBA32Uint: return MTL::PixelFormatRGBA32Uint;
    case PixelType::R32Float: return MTL::PixelFormatR32Float;
    case PixelType::RG32Float: return MTL::PixelFormatRG32Float;
    case PixelType::RGBA32Float: return MTL::PixelFormatRGBA32Float;
    case PixelType::Depth32Float: return MTL::PixelFormatDepth32Float;
    case PixelType::Depth24Stencil8: return MTL::PixelFormatDepth24Unorm_Stencil8;
    default: 
      break;
  }

  return MTL::PixelFormatInvalid;
}

static PixelType MTLPixelFormatToPixelType(MTL::PixelFormat type)
{
  switch (type)
  {
    case MTL::PixelFormatR8Unorm: return PixelType::R8;
    case MTL::PixelFormatRG8Unorm: return PixelType::RG8;
    case MTL::PixelFormatRGBA8Unorm: return PixelType::RGBA8;
    case MTL::PixelFormatBGRA8Unorm: return PixelType::BGRA8;
    case MTL::PixelFormatR16Unorm: return PixelType::R16;
    case MTL::PixelFormatRG16Unorm: return PixelType::RG16;
    case MTL::PixelFormatRGBA16Unorm: return PixelType::RGBA16;
    case MTL::PixelFormatR16Float: return PixelType::R16Float;
    case MTL::PixelFormatRG16Float: return PixelType::RG16Float;
    case MTL::PixelFormatRGBA16Float: return PixelType::RGBA16Float;
    case MTL::PixelFormatR32Uint: return PixelType::R32Uint;
    case MTL::PixelFormatRG32Uint: return PixelType::RG32Uint;
    case MTL::PixelFormatRGBA32Uint: return PixelType::RGBA32Uint;
    case MTL::PixelFormatR32Float: return PixelType::R32Float;
    case MTL::PixelFormatRG32Float: return PixelType::RG32Float;
    case MTL::PixelFormatRGBA32Float: return PixelType::RGBA32Float;
    case MTL::PixelFormatDepth32Float: return PixelType::Depth32Float;
    case MTL::PixelFormatDepth24Unorm_Stencil8: return PixelType::Depth24Stencil8;
    default:
      break;
  }

  return PixelType::Invalid;
}

static MTL::VertexFormat ShaderDataTypeToMTLVertexFormat(ShaderDataType type, bool normalized = false)
{
  if (!normalized)
  {
    switch (type)
    {
      case ShaderDataType::Int: return MTL::VertexFormatInt;
      case ShaderDataType::Float: return MTL::VertexFormatFloat;
      case ShaderDataType::Float2: return MTL::VertexFormatFloat2;
      case ShaderDataType::Float3: return MTL::VertexFormatFloat3;
      case ShaderDataType::Float4: return MTL::VertexFormatFloat4;
      case ShaderDataType::UByte4: return MTL::VertexFormatUChar4;
      default:
        break;
    }

    std::cout << "Unknown Shader Data Type!" << std::endl;
    return MTL::VertexFormatInvalid;
  } 
  else
  {
    switch (type)
    {
      case ShaderDataType::UByte4: return MTL::VertexFormatUChar4Normalized;
      default:
        break;
    }

    std::cout << "Unknown Shader Data Type!" << std::endl;
    return MTL::VertexFormatInvalid;
  }

}

static PixelType ChannelsToPixelType(int channels)
{
  switch (channels)
  {
    case 1: return PixelType::R8;
    case 2: return PixelType::RG8;
    case 3:
    case 4:
    default:
      return PixelType::RGBA8;
  }
}

static MTL::CompareFunction DepthFunctionToMTLCompareFunction(DepthFunc func)
{
  switch (func)
  {
    case DepthFunc::Less: return MTL::CompareFunctionLess;
    case DepthFunc::LessEqual: return MTL::CompareFunctionLessEqual;
    case DepthFunc::Greater: return MTL::CompareFunctionGreater;
    case DepthFunc::GreaterEqual: return MTL::CompareFunctionGreaterEqual;
    case DepthFunc::Equal: return MTL::CompareFunctionEqual;
    default:
      return MTL::CompareFunctionAlways;
  }
}

static MTL::LoadAction LoadOpToMTLLoadAction(LoadOp op)
{
  switch (op)
  {
    case LoadOp::Load: return MTL::LoadActionLoad;
    case LoadOp::Clear: return MTL::LoadActionClear;
    case LoadOp::DontCare: return MTL::LoadActionDontCare;
    default:
      return MTL::LoadActionDontCare;
  }
}

static MTL::StoreAction StoreOpToMTLStoreAction(StoreOp op)
{
  switch (op)
  {
    case StoreOp::DontCare: return MTL::StoreActionDontCare;
    case StoreOp::Store: return MTL::StoreActionStore;
    default:
      return MTL::StoreActionDontCare;
  }
}

static MTL::IndexType IndexTypeToMTLIndexType(IndexType type)
{
  switch (type)
  {
    case IndexType::U16: return MTL::IndexTypeUInt16;
    case IndexType::U32: return MTL::IndexTypeUInt32;
    case IndexType::U8: break;
    default:
      break;
  }

  std::cout << "Unsupported Index Type in Metal: Use U16 or U32" << std::endl;
  return MTL::IndexTypeUInt16;
}

static int PixelTypeToChannels(PixelType type)
{
  switch (type)
  {
    case PixelType::R8: return 1;
    case PixelType::RG8: return 2;
    case PixelType::RGBA8: return 4;
    case PixelType::BGRA8: return 4;
    case PixelType::R16: return 1;
    case PixelType::RG16: return 2;
    case PixelType::RGBA16: return 4;
    case PixelType::R16Float: return 1;
    case PixelType::RG16Float: return 2;
    case PixelType::RGBA16Float: return 4;
    case PixelType::R32Uint: return 1;
    case PixelType::RG32Uint: return 2;
    case PixelType::RGBA32Uint: return 4;
    case PixelType::R32Float: return 1;
    case PixelType::RG32Float: return 2;
    case PixelType::RGBA32Float: return 4;
    case PixelType::Depth32Float: return 1;
    case PixelType::Depth24Stencil8: return 2;

    default: break;
  }
  
  std::cout << "Unknown PixelType" << std::endl;
  return -1;
}

static MTL::SamplerMinMagFilter MinMagFilterToMTLSamplerMinMagFilter (MinMagFilter filter)
{
  switch (filter)
  {
    case MinMagFilter::Nearest: return MTL::SamplerMinMagFilterNearest;
    case MinMagFilter::Linear: return MTL::SamplerMinMagFilterLinear;
  }
}

}
