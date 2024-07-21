#include "GLCompiler.h"

#include <spirv_glsl.hpp>
#include <iostream>

#include "GLTypes.h"

namespace Vision
{

GLuint GLCompiler::Compile(const ShaderSPIRV& shader, uint32_t version)
{
  // Decompile to GLSL
  spirv_cross::CompilerGLSL decompiler(shader.SPIRV);
  
  spirv_cross::CompilerGLSL::Options options;
  options.version = version;
  options.enable_420pack_extension = false;
  decompiler.set_common_options(options);

  std::string glsl = decompiler.compile();
  const char* c_str = glsl.c_str();

  // Create the shader
  GLuint shaderID = glCreateShader(ShaderStageToGLenum(shader.Stage));
  glShaderSource(shaderID, 1, &c_str, nullptr);
  glCompileShader(shaderID);

  int success;
  glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    constexpr static std::size_t bufferSize = 512;
    char infoLog[bufferSize];

    glGetShaderInfoLog(shaderID, bufferSize, nullptr, infoLog);
    std::cout << "Failed to compile shader: " << shader.Name << std::endl;
    std::cout << infoLog << std::endl;
    return 0;
  }

  return shaderID;
}

}