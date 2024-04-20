#pragma once

#include <glad/glad.h>

#include "renderer/primitive/Shader.h"
#include "renderer/primitive/Buffer.h"

namespace Vision
{

class GLProgram
{
public:
  GLProgram();
  GLProgram(const std::unordered_map<ShaderStage, std::string> &shaders);
  ~GLProgram();

  void Use();

  GLuint GetProgram() const { return program; }
  bool UsesTesselation() const { return usesTesselation; }

  void UploadUniformInt(const int, const char *name);
  void UploadUniformIntArray(const int *, std::size_t numElements, const char *name);
  void UploadUniformFloat(const float, const char *name);
  void UploadUniformFloat2(const float *, const char *name);
  void UploadUniformFloat3(const float *, const char *name);
  void UploadUniformFloat4(const float *, const char *name);
  void UploadUniformMat4(const float *, const char *name);

  void SetUniformBlock(Buffer *buffer, const char *name, std::size_t binding);

private:
  GLuint program;
  bool usesTesselation = false;
};

}