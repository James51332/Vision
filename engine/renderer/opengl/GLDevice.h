#pragma once

#include "renderer/primitive/APIDevice.h"
#include "renderer/primitive/ObjectCache.h"

#include "GLPipeline.h"
#include "GLProgram.h"
#include "GLVertexArray.h"
#include "GLBuffer.h"

namespace Vision
{

class GLDevice : public APIDevice
{
public:
  ID CreatePipeline(const PipelineDesc& desc);
  GLPipeline* GetPipeline(ID pipeline) { return pipelines.Get(pipeline); }
  void DestroyPipeline(ID pipeline) { pipelines.Destroy(pipeline); }

  ID CreateShader(const ShaderDesc &desc);
  GLProgram* GetShader(ID shader) { return shaders.Get(shader); }
  void DestroyShader(ID shader) { shaders.Destroy(shader); }

  ID CreateBuffer(const BufferDesc& desc);
  void SetBufferData(ID buffer, void* data, std::size_t size) { buffers.Get(buffer)->SetData(data, size); }
  void ResizeBuffer(ID buffer, std::size_t size) { buffers.Get(buffer)->Resize(size); }
  void AttachUniformBuffer(ID buffer, std::size_t block = 0) { buffers.Get(buffer)->Attach(block); }
  GLBuffer* GetBuffer(ID buffer) { return buffers.Get(buffer); }
  void DestroyBuffer(ID id) { buffers.Destroy(id); }

  virtual void Submit(const DrawCommand &command);

private:
  ObjectCache<GLPipeline> pipelines;
  ObjectCache<GLProgram> shaders;
  ObjectCache<GLBuffer> buffers;
  GLVertexArrayCache vaoCache;

  std::size_t currentID = 1;
};

}