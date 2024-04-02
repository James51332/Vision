#include "Shader.h"

#include <iostream>
#include <sstream>

#include <SDL.h>

namespace Vision
{

static GLenum ShaderTypeFromString(const std::string &type)
{
  if (type == "vertex")
    return GL_VERTEX_SHADER;
  if (type == "fragment" || type == "pixel")
    return GL_FRAGMENT_SHADER;
  if (type == "tesselation_control" || type == "tcs")
    return GL_TESS_CONTROL_SHADER;
  if (type == "tesselation_evaluation" || type == "tes")
    return GL_TESS_EVALUATION_SHADER;

  SDL_assert(false);
  return 0;
}

Shader::Shader(const char* path)
{
  SDL_RWops *shader = SDL_RWFromFile(path, "r+");
  if (!shader)
  {
    SDL_Log("Failed to open shader: %s", SDL_GetError());
  }

  size_t size = SDL_RWsize(shader);
  std::string buffer(size, ' ');
  SDL_RWread(shader, &buffer[0], size);

  std::unordered_map<GLenum, std::string> shaders;

  // System from TheCherno/Hazel
  const char* typeToken = "#type";
  size_t typeTokenLength = strlen(typeToken);
  std::size_t pos = buffer.find(typeToken, 0);
  while (pos != std::string::npos)
  {
    size_t eol = buffer.find_first_of("\r\n", pos); // End of shader type declaration line
    SDL_assert(eol != std::string::npos);
    size_t begin = pos + typeTokenLength + 1; // Start of shader type name (after "#type " keyword)
    std::string type = buffer.substr(begin, eol - begin);

    size_t nextLinePos = buffer.find_first_not_of("\r\n", eol); // Start of shader code after shader type declaration line
    SDL_assert(nextLinePos != std::string::npos);
    pos = buffer.find(typeToken, nextLinePos); // Start of next shader type declaration line

    shaders[ShaderTypeFromString(type)] = (pos == std::string::npos) ? buffer.substr(nextLinePos) : buffer.substr(nextLinePos, pos - nextLinePos);
  }

  SDL_assert(shaders.size() == 2);
  SDL_assert(shaders[GL_VERTEX_SHADER].size() != 0);
  SDL_assert(shaders[GL_FRAGMENT_SHADER].size() != 0);
  CreateFromSources(shaders);
}

Shader::Shader(const char* vertex, const char* fragment)
{
  std::unordered_map<GLenum, std::string> shaders;
  shaders[GL_VERTEX_SHADER] = vertex;
  shaders[GL_FRAGMENT_SHADER] = fragment;

  CreateFromSources(shaders);
}

void Shader::CreateFromSources(std::unordered_map<GLenum, std::string>& shaders)
{
  std::unordered_map<GLenum, GLuint> shaderIDs;
  
  // Compile all shaders
  for (auto pair : shaders)
  {
    GLenum type = pair.first;
    std::string text = pair.second;
    const char *c_str = text.c_str();

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
      std::cout << "Failed to compile shader:" << std::endl;
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
  m_ShaderProgram = glCreateProgram();
  {
    for (auto pair : shaderIDs)
      glAttachShader(m_ShaderProgram, pair.second);

    glLinkProgram(m_ShaderProgram);

    int success;
    glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
      constexpr static std::size_t bufferSize = 512;
      char infoLog[bufferSize];

      glGetProgramInfoLog(m_ShaderProgram, bufferSize, nullptr, infoLog);
      std::cout << "Failed to link shader program:" << std::endl;
      std::cout << infoLog << std::endl;
    }
  }

  // delete our shaders now that we have linked
  for (auto pair : shaderIDs)
    glDeleteShader(pair.second);
}

Shader::~Shader()
{
  glDeleteProgram(m_ShaderProgram);
}

void Shader::Use()
{
  glUseProgram(m_ShaderProgram);
}

void Shader::UploadUniformInt(const int value, const char* name)
{
  GLint location = glGetUniformLocation(m_ShaderProgram, name);
  glUniform1i(location, value);
}

void Shader::UploadUniformIntArray(const int* value, std::size_t numElements, const char* name)
{
  GLint location = glGetUniformLocation(m_ShaderProgram, name);
  glUniform1iv(location, static_cast<GLsizei>(numElements), value);
}

void Shader::UploadUniformFloat(const float value, const char* name)
{
  GLint location = glGetUniformLocation(m_ShaderProgram, name);
  glUniform1f(location, value);
}

void Shader::UploadUniformFloat3(const float* value, const char* name)
{
  GLint location = glGetUniformLocation(m_ShaderProgram, name);
  glUniform3fv(location, 1, value);
}

void Shader::UploadUniformFloat4(const float* value, const char *name)
{
  GLint location = glGetUniformLocation(m_ShaderProgram, name);
  glUniform4fv(location, 1, value);
}

void Shader::UploadUniformMat4(const float* value, const char *name)
{
  GLint location = glGetUniformLocation(m_ShaderProgram, name);
  glUniformMatrix4fv(location, 1, GL_FALSE, value);
}

void Shader::SetUniformBlock(Buffer* uniform, const char* name, std::size_t binding)
{
  unsigned int waves_index = glGetUniformBlockIndex(m_ShaderProgram, name);
  glUniformBlockBinding(m_ShaderProgram, waves_index, binding);
  glBindBufferBase(GL_UNIFORM_BUFFER, binding, uniform->m_Object);
}

}