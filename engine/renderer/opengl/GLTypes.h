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
  }
}

static GLenum PrimitiveTypeToGLenum(PrimitiveType type)
{
  switch (type)
  {
    case PrimitiveType::Triangle: return GL_TRIANGLES;
    case PrimitiveType::TriangleStrip: return GL_TRIANGLE_STRIP;
    case PrimitiveType::Patch: return GL_PATCHES;
  }
}

static const char* ShaderStageToString(ShaderStage type)
{
  switch (type)
  {
    case ShaderStage::Vertex: return "vertex";
    case ShaderStage::Pixel: return "pixel";
    case ShaderStage::Domain: return "domain";
    case ShaderStage::Hull: return "hull";
    case ShaderStage::Geometry: return "geometry";
    default:
      return "unknown";
  }
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
  }

  SDL_assert(false);
  return 0;
}

static GLenum BufferTypeToGLenum(BufferType type)
{
  switch (type)
  {
    case BufferType::Vertex: return GL_ARRAY_BUFFER;
    case BufferType::Index: return GL_ELEMENT_ARRAY_BUFFER;
    case BufferType::Uniform: return GL_UNIFORM_BUFFER;
  }

  SDL_assert(false);
  return 0;
}

static GLenum BufferUsageToGLenum(BufferUsage usage)
{
  switch (usage)
  {
    case BufferUsage::Static: return GL_STATIC_DRAW;
    case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW;
  }

  SDL_assert(false);
  return 0;
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

static GLenum PixelTypeToGLInternalFormat(PixelType type)
{
  switch (type)
  {
    case PixelType::R8: return GL_R8;
    case PixelType::RG16: return GL_RG8;
    case PixelType::RGB24: return GL_RGB8;
    case PixelType::RGBA32: return GL_RGBA8;
    case PixelType::Depth32: return GL_DEPTH_COMPONENT32;
    case PixelType::Depth24Stencil8: return GL_DEPTH24_STENCIL8;
  }
  
  SDL_assert(false);
  return 0;
}

static GLenum PixelTypeTOGLFormat(PixelType type)
{
  switch (type)
  {
    case PixelType::R8: return GL_RED;
    case PixelType::RG16: return GL_RG;
    case PixelType::RGB24: return GL_RGB;
    case PixelType::RGBA32: return GL_RGBA;
    case PixelType::Depth32: return GL_DEPTH_COMPONENT;
    case PixelType::Depth24Stencil8: return GL_DEPTH_STENCIL;
  }

  SDL_assert(false);
  return 0;
}

}