#pragma once

#include "renderer/primitive/APIDevice.h"
#include "renderer/primitive/ObjectCache.h"

#include "GLPipeline.h"
#include "GLProgram.h"
#include "GLVertexArray.h"

namespace Vision
{

class GLDevice : public APIDevice
{
public:
  GLDevice();
  ~GLDevice();

  ID CreatePipeline(const PipelineDesc& desc);
  void DestroyPipeline(ID pipeline);

  ID CreateShader(const ShaderDesc &desc);
  void DestroyShader(ID id);

  virtual void Submit(const DrawCommand &command);

private:
  ObjectCache<GLPipeline> pipelines;
  ObjectCache<GLProgram> shaders;
  GLVertexArrayCache vaoCache;

  std::size_t currentID = 1;
};

}