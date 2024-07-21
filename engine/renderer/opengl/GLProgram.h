#pragma once

#include <glad/glad.h>
#include <unordered_map>

#include "renderer/primitive/Buffer.h"
#include "renderer/primitive/Pipeline.h"

#include "renderer/shader/Shader.h"

namespace Vision
{

// ----- GLProgram -----

class GLProgram
{
public:
  GLProgram() = default;
  GLProgram(const ShaderSPIRV& vertexShader, const ShaderSPIRV& fragmentShader, bool manualBinding);
  ~GLProgram();

  void Use();
  GLuint GetProgram() const { return program; }

  void SetUniformBlock(const char *name, std::size_t binding);

  void UploadUniformInt(const int, const char *name);
  void UploadUniformIntArray(const int *, std::size_t numElements, const char *name);
  void UploadUniformFloat(const float, const char *name);
  void UploadUniformFloat2(const float *, const char *name);
  void UploadUniformFloat3(const float *, const char *name);
  void UploadUniformFloat4(const float *, const char *name);
  void UploadUniformMat4(const float *, const char *name);

private:
  void Reflect(const ShaderSPIRV& shader);

private:
  GLuint program = 0;
};

// ----- GLComputeProgram -----

// In OpenGL, there are no such things as pipelines. However, we do emulate the concept
// in the rendering pipeline because there is no data structure pipelines could exist as.
// However, for compute pipelines, we can simply use a GLProgram, since compute dispatches
// only require the program in OpenGL, whereas other APIs only require the pipeline state.

class GLComputeProgram
{
public:
  GLComputeProgram(const std::vector<ShaderSPIRV>& kernels);
  ~GLComputeProgram();

  void Use(const std::string& kernel);

private:
  std::unordered_map<std::string, GLuint> programs;
};

}