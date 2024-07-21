#include "GLProgram.h"

#include <iostream>
#include <sstream>
#include <SDL.h>

#include "GLTypes.h"
#include "GLCompiler.h"

#include "renderer/shader/ShaderReflector.h"

namespace Vision
{

// ----- GLProgram -----

GLProgram::GLProgram(const ShaderSPIRV& vertexShader, const ShaderSPIRV& fragmentShader, bool manualBindings)
  : program(0)
{
  GLCompiler compiler;

  uint32_t version = manualBindings ? 410 : 450;
  GLuint vs = compiler.Compile(vertexShader, version);
  GLuint fs = compiler.Compile(fragmentShader, version);

  // attach the shaders to a program and link it
  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);

  int success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success)
  {
    constexpr static std::size_t bufferSize = 512;
    char infoLog[bufferSize];
    glGetProgramInfoLog(program, bufferSize, nullptr, infoLog);

    std::cout << "Failed to link shader program with shaders: " << vertexShader.Name << ", " << fragmentShader.Name  << std::endl;
    std::cout << infoLog << std::endl;
  }

  // delete our shaders now that we have linked
  glDeleteShader(vs);
  glDeleteShader(fs);

  // use a reflector to attach what we can
  if (manualBindings)
  {
    // TODO: Check for collision of binding slots between shader stages.
    Reflect(vertexShader);
    Reflect(fragmentShader);
  }
}

void GLProgram::Reflect(const ShaderSPIRV& shader)
{
  ShaderReflector reflector(shader);
  
  auto images = reflector.GetSampledImages();
  for (auto image : images)
  {
    if (image.ArraySize == 1)
    {
      UploadUniformInt(image.Binding, image.Name.c_str());
    }
    else
    {
      std::vector<int> bindings(image.ArraySize, image.Binding);
      for (int i = 0; i < image.ArraySize; i++)
        bindings[i] += i;

      UploadUniformIntArray(bindings.data(), image.ArraySize, image.Name.c_str());
    }
  }

  auto ubos = reflector.GetUniformBuffers();
  for (auto ubo : ubos)
    SetUniformBlock(ubo.Name.c_str(), ubo.Binding);
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
GLComputeProgram::GLComputeProgram(const std::vector<ShaderSPIRV>& computeKernels)
{
  GLCompiler compiler;

  for (auto kernel : computeKernels)
  {
    if (kernel.Stage != ShaderStage::Compute)
    {
      std::cout << "Compute pipelines can only be built with type(compute) shaders" << std::endl;
      continue;
    }

    GLuint shader = compiler.Compile(kernel);
    if (!shader)
      continue;

    GLuint program = glCreateProgram();
    glAttachShader(program, shader);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
      constexpr static std::size_t bufferSize = 512;
      char infoLog[bufferSize];
      glGetProgramInfoLog(program, bufferSize, nullptr, infoLog);

      std::cout << "Failed to link shader program with shader: " << kernel.Name << std::endl;
      std::cout << infoLog << std::endl;
    }

    programs[kernel.Name] = program;
    glDeleteShader(shader);
  }
}

GLComputeProgram::~GLComputeProgram()
{
  for (auto pair : programs)
    glDeleteProgram(pair.second);
}

void GLComputeProgram::Use(const std::string& kernel)
{
  glUseProgram(programs[kernel]);
}

}