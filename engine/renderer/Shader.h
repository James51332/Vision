#pragma once

#include <glad/glad.h>

#include "Buffer.h"

namespace Vision
{

class Shader
{
public:
  Shader(const char* path);
  Shader(const char* vertex, const char* fragment);
  ~Shader();

  void Use();

  void UploadUniformInt(const int, const char* name);
  void UploadUniformFloat(const float, const char* name);
  void UploadUniformFloat3(const float*, const char* name);
  void UploadUniformFloat4(const float*, const char *name);
  void UploadUniformMat4(const float*, const char *name);

  void SetUniformBlock(Buffer* buffer, const char* name, std::size_t binding);

private:
  void CreateFromSources(const char* vertex, const char* fragment); 

private:
  GLuint m_ShaderProgram;
};

}