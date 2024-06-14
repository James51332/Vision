#pragma once

#include <Metal/Metal.hpp>
#include <Metal/MTLPixelFormat.hpp>

#include <iostream>

#include "renderer/primitive/Texture.h"
#include "renderer/primitive/Buffer.h"
#include "renderer/primitive/Pipeline.h"
#include "renderer/primitive/RenderPass.h"

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

static PixelType ChannelsToPixelType(int channels)
{
  switch (channels)
  {
    case 1: return PixelType::R8;
    case 2: return PixelType::RG16;
    case 3: return PixelType::RGB24;
    case 4:
    default:
      return PixelType::RGBA32;
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

}