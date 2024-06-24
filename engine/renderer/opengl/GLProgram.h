#pragma once

#include <glad/glad.h>

#include "renderer/primitive/Shader.h"
#include "renderer/primitive/Buffer.h"

// Compute pipelines in OpenGL map to GLComputeProgram
#include "renderer/primitive/Pipeline.h"

namespace Vision
{

// ----- GLProgram -----

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

  void SetUniformBlock(const char *name, std::size_t binding);

protected:
  void CreateProgramFromMap(const std::unordered_map<ShaderStage, std::string>& shaders);

private:
  GLuint program;
  bool usesTesselation = false;
};

// ----- GLComputeProgram -----

// In OpenGL, there are no such things as pipelines. However, we do emulate the concept
// in the rendering pipeline because there is no data structure pipelines could exist as.
// However, for compute pipelines, we can simply use a GLProgram, since compute dispatches
// only require the program in OpenGL, whereas other APIs only require the pipeline state.

class GLComputeProgram : public GLProgram
{
public:
  GLComputeProgram(const ComputePipelineDesc& desc);
};

}