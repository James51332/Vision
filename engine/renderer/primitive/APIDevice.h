#pragma once 

#include "renderer/primitive/Pipeline.h"
#include "renderer/primitive/Buffer.h"
#include "renderer/primitive/Texture.h"
#include "renderer/primitive/Shader.h"

#include "renderer/RenderCommand.h"

namespace Vision
{

using ID = std::size_t;

class APIDevice
{
public:
  virtual ~APIDevice() {}

  virtual ID CreatePipeline(const PipelineDesc& desc) = 0;
  virtual void DestroyPipeline(ID id) = 0;

  virtual ID CreateShader(const ShaderDesc& desc) = 0;
  virtual void DestroyShader(ID id) = 0;

  virtual ID CreateBuffer(const BufferDesc &desc) = 0;
  virtual void SetBufferData(ID buffer, void *data, std::size_t size) = 0;
  virtual void ResizeBuffer(ID buffer, std::size_t size) = 0;
  virtual void AttachUniformBuffer(ID buffer, std::size_t block = 0) = 0; 
  virtual void DestroyBuffer(ID id) = 0;

  virtual void Submit(const DrawCommand& command) = 0;
};

}