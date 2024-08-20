#pragma once

#include <glad/glad.h>
#include <SDL.h>

#include "renderer/RenderCommand.h"

namespace Vision
{

static GLenum IndexTypeToGLenum(IndexType type)
{
  switch (type)
  {
    case IndexType::U8: return GL_UNSIGNED_BYTE;
    case IndexType::U16: return GL_UNSIGNED_SHORT;
    case IndexType::U32: return GL_UNSIGNED_INT;
    default:
      break;
  }

  return GL_INVALID_ENUM;
}

static GLenum PrimitiveTypeToGLenum(PrimitiveType type)
{
  switch (type)
  {
    case PrimitiveType::Triangle: return GL_TRIANGLES;
    case PrimitiveType::TriangleStrip: return GL_TRIANGLE_STRIP;
    case PrimitiveType::Patch: return GL_PATCHES;
    default:
      break;
  }

  return GL_INVALID_ENUM;
}

static GLenum ShaderStageToGLenum(ShaderStage stage)
{
  switch (stage)
  {
    case ShaderStage::Vertex: return GL_VERTEX_SHADER;
    case ShaderStage::Pixel: return GL_FRAGMENT_SHADER;
    case ShaderStage::Hull: return GL_TESS_CONTROL_SHADER;
    case ShaderStage::Domain: return GL_TESS_EVALUATION_SHADER;
    case ShaderStage::Geometry: return GL_GEOMETRY_SHADER;
    case ShaderStage::Compute: return GL_COMPUTE_SHADER;
    default:
      break;
  }

  return GL_INVALID_ENUM;
}

static GLenum BufferTypeToGLenum(BufferType type)
{
  switch (type)
  {
    case BufferType::Vertex: return GL_ARRAY_BUFFER;
    case BufferType::Index: return GL_ELEMENT_ARRAY_BUFFER;
    case BufferType::Uniform: return GL_UNIFORM_BUFFER;
    case BufferType::ShaderStorage: return GL_SHADER_STORAGE_BUFFER;
  }

  return GL_INVALID_ENUM;
}

static GLenum BufferUsageToGLenum(BufferUsage usage)
{
  switch (usage)
  {
    case BufferUsage::Static: return GL_STATIC_DRAW;
    case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW;
  }

  return GL_INVALID_ENUM;
}

static PixelType ChannelsToPixelType(int channels)
{
  switch (channels)
  {
    case 1: return PixelType::R8;
    case 2: return PixelType::RG8;
    case 4:
    default:
      return PixelType::RGBA8;
  }
}

static GLenum PixelTypeToGLInternalFormat(PixelType type)
{
  switch (type)
  {
    case PixelType::R8: return GL_R8;
    case PixelType::RG8: return GL_RG8;
    case PixelType::RGBA8: return GL_RGBA8;
    case PixelType::BGRA8: return GL_RGBA8;
    case PixelType::R16: return GL_R16;
    case PixelType::RG16: return GL_RG16;
    case PixelType::RGBA16: return GL_RGBA16;
    case PixelType::R16Float: return GL_R16F;
    case PixelType::RG16Float: return GL_RG16F;
    case PixelType::RGBA16Float: return GL_RGBA16F;
    case PixelType::R32Uint: return GL_R32UI;
    case PixelType::RG32Uint: return GL_RG32UI;
    case PixelType::RGBA32Uint: return GL_RGBA32UI;
    case PixelType::R32Float: return GL_R32F;
    case PixelType::RG32Float: return GL_RG32F;
    case PixelType::RGBA32Float: return GL_RGBA32F;
    case PixelType::Depth32Float: return GL_DEPTH_COMPONENT32F;
    case PixelType::Depth24Stencil8: return GL_DEPTH24_STENCIL8;
    default:
      break;
  }

  return GL_INVALID_ENUM;
}

static GLenum PixelTypeToGLFormat(PixelType type)
{
  switch (type)
  {
    case PixelType::R8: return GL_RED;
    case PixelType::RG8: return GL_RG;
    case PixelType::RGBA8: return GL_RGBA;
    case PixelType::BGRA8: return GL_BGRA;
    case PixelType::R16: return GL_RED;
    case PixelType::RG16: return GL_RG;
    case PixelType::RGBA16: return GL_RGBA;
    case PixelType::R16Float: return GL_RED;
    case PixelType::RG16Float: return GL_RG;
    case PixelType::RGBA16Float: return GL_RGBA;
    case PixelType::R32Uint: return GL_RED;
    case PixelType::RG32Uint: return GL_RG;
    case PixelType::RGBA32Uint: return GL_RGBA;
    case PixelType::R32Float: return GL_RED;
    case PixelType::RG32Float: return GL_RG;
    case PixelType::RGBA32Float: return GL_RGBA;
    case PixelType::Depth32Float: return GL_DEPTH_COMPONENT;
    case PixelType::Depth24Stencil8: return GL_DEPTH_COMPONENT;
    default: break;
  }

  return GL_INVALID_ENUM;
}

static GLenum PixelTypeToGLType(PixelType type)
{
  switch (type)
  {
    case PixelType::R8:
    case PixelType::RG8: 
    case PixelType::RGBA8:
    case PixelType::BGRA8: return GL_UNSIGNED_BYTE;
    case PixelType::R16: 
    case PixelType::RG16: 
    case PixelType::RGBA16: return GL_UNSIGNED_SHORT;
    case PixelType::R16Float:
    case PixelType::RG16Float:
    case PixelType::RGBA16Float: return GL_FLOAT;
    case PixelType::R32Uint: 
    case PixelType::RG32Uint:
    case PixelType::RGBA32Uint: return GL_UNSIGNED_INT;
    case PixelType::R32Float: 
    case PixelType::RG32Float: 
    case PixelType::RGBA32Float: 
    case PixelType::Depth32Float: return GL_FLOAT;
    default: break;
  }

  return GL_INVALID_ENUM;
}

static GLenum DepthFuncToGLenum(DepthFunc func)
{
  switch (func)
  {
    case DepthFunc::Less: return GL_LESS;
    case DepthFunc::LessEqual: return GL_LEQUAL;
    case DepthFunc::Greater: return GL_GREATER;
    case DepthFunc::GreaterEqual: return GL_GEQUAL;
    case DepthFunc::Equal: return GL_EQUAL;
    default:
      break;
  }

  return GL_INVALID_ENUM;
}

static GLenum MinMagFilterToGLenum(MinMagFilter filter)
{
  switch (filter)
  {
    case MinMagFilter::Nearest: return GL_NEAREST;
    case MinMagFilter::Linear: return GL_LINEAR;
  }
}

}