#pragma once

#include <glad/glad.h>

#include "GLProgram.h"

namespace Vision
{

struct GLPipeline
{
  GLProgram* Program;
  std::vector<BufferLayout> Layouts;

  bool DepthTest;
  bool DepthWrite;
  GLenum DepthFunc;

  bool EnableBlend;
  GLenum BlendSource;
  GLenum BlendDst;
};

}