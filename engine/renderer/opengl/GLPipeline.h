#pragma once

#include "GLProgram.h"

namespace Vision
{

struct GLPipeline
{
  GLProgram* Shader;
  std::vector<BufferLayout> Layouts;
};

}