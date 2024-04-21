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

}