#pragma once

#include <glad/glad.h>

#include "renderer/shader/Shader.h"

namespace Vision
{

class GLCompiler
{
public:
  GLuint Compile(const ShaderSPIRV& shaderSPIRV, uint32_t version = 450);
};

}