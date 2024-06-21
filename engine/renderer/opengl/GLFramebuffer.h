#pragma once

#include "GLTexture.h"

#include "renderer/primitive/Framebuffer.h"

namespace Vision
{

class GLFramebuffer
{
public:
  GLFramebuffer(const FramebufferDesc& desc);
  ~GLFramebuffer();

  void Resize(float width, float height);

  GLTexture2D* GetColorAttachment() { return colorAttachment; }
  GLTexture2D* GetDepthAttachment() { return depthStencilAttachment; }
  const FramebufferDesc& GetDesc() const { return desc; }

  // If a framebuffer is bound, all rendering will occur on that framebuffer
  void Bind();
  void Unbind();

private:
  GLuint framebufferID;
  FramebufferDesc desc;

  GLTexture2D* colorAttachment;
  GLTexture2D* depthStencilAttachment;
};

}
