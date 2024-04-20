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

  virtual void Submit(const DrawCommand& command) = 0;
};

}