#include "GLProgram.h"

#include <iostream>
#include <sstream>
#include <SDL.h>

#include "GLTypes.h"

#include "renderer/ShaderCompiler.h"

namespace Vision
{

// ----- GLProgram -----

GLProgram::GLProgram()
  : program(0), usesTesselation(false)
{
}

void GLProgram::CreateProgramFromMap(const std::unordered_map<ShaderStage, std::string>& shaders)
{
  std::unordered_map<GLenum, GLuint> shaderIDs;

  // Compile all shaders
  for (auto pair : shaders)
  {
    GLenum type = ShaderStageToGLenum(pair.first);
    std::string text = pair.second;
    const char* c_str = text.c_str();

    // give the source code to gl compiler
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &c_str, nullptr);
    glCompileShader(id);

    // check compile status
    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      constexpr static std::size_t bufferSize = 512;
      char infoLog[bufferSize];

      glGetShaderInfoLog(id, bufferSize, nullptr, infoLog);
      std::cout << "Failed to compile shader: " << ShaderStageToString(pair.first) << std::endl;
      std::cout << infoLog << std::endl;
    }

    // insert the id into our map
    if (shaderIDs[type])
    {
      SDL_assert(false);
      std::cout << "Failed to link program! Cannot have more than one of each shader type!" << std::endl;
    }

    shaderIDs[type] = id;
  }

  // attach the shaders to a program and link it
  program = glCreateProgram();
  {
    for (auto pair : shaderIDs)
      glAttachShader(program, pair.second);

    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
      constexpr static std::size_t bufferSize = 512;
      char infoLog[bufferSize];

      glGetProgramInfoLog(program, bufferSize, nullptr, infoLog);
      std::cout << "Failed to link shader program:" << std::endl;
      std::cout << infoLog << std::endl;
    }
  }

  // if our shader worked, and we have a tes, then we're a tesselation shader
  if (shaderIDs[GL_TESS_EVALUATION_SHADER])
    usesTesselation = true;

  // delete our shaders now that we have linked
  for (auto pair : shaderIDs)
    glDeleteShader(pair.second);
}

GLProgram::GLProgram(const std::unordered_map<ShaderStage, std::string>& shaders)
{
  CreateProgramFromMap(shaders);
}

GLProgram::~GLProgram()
{
  glDeleteProgram(program);
}

void GLProgram::Use()
{
  glUseProgram(program);
}

void GLProgram::UploadUniformInt(const int value, const char* name)
{
  GLint location = glGetUniformLocation(program, name);
  glUniform1i(location, value);
}

void GLProgram::UploadUniformIntArray(const int* value, std::size_t numElements, const char* name)
{
  GLint location = glGetUniformLocation(program, name);
  glUniform1iv(location, static_cast<GLsizei>(numElements), value);
}

void GLProgram::UploadUniformFloat(const float value, const char* name)
{
  GLint location = glGetUniformLocation(program, name);
  glUniform1f(location, value);
}

void GLProgram::UploadUniformFloat2(const float* value, const char* name)
{
  GLint location = glGetUniformLocation(program, name);
  glUniform2fv(location, 1, value);
}

void GLProgram::UploadUniformFloat3(const float* value, const char* name)
{
  GLint location = glGetUniformLocation(program, name);
  glUniform3fv(location, 1, value);
}

void GLProgram::UploadUniformFloat4(const float* value, const char *name)
{
  GLint location = glGetUniformLocation(program, name);
  glUniform4fv(location, 1, value);
}

void GLProgram::UploadUniformMat4(const float* value, const char *name)
{
  GLint location = glGetUniformLocation(program, name);
  glUniformMatrix4fv(location, 1, GL_FALSE, value);
}

void GLProgram::SetUniformBlock(const char* name, std::size_t binding)
{
  unsigned int waves_index = glGetUniformBlockIndex(program, name);
  glUniformBlockBinding(program, waves_index, binding);
}

// ----- GLComputeProgram -----
GLComputeProgram::GLComputeProgram(const ComputePipelineDesc& desc)
{
  ShaderCompiler compiler;
  std::string source;

  // Read the shader code from any valid source
  if (desc.Source == ShaderInput::File)
    source = compiler.ReadFile(desc.FilePath);
  else if (desc.Source == ShaderInput::GLSL)
    source = desc.GLSL;
  else
  {
    std::cout << "Invalid Compute Shader Source!" << std::endl;
    SDL_assert(false);
  }

  // Parse the shader code and ensure it's a valid compute shader
  auto map = compiler.Parse(source);
  SDL_assert(map.size() == 1);
  SDL_assert(map[ShaderStage::Compute].size() != 0);

  // Create the program from the map using parent method
  CreateProgramFromMap(map);
}

}