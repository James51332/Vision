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

  ID GetColorID() const { return colorID; }
  ID GetDepthID() const { return depthID; }
  void SetColorID(ID id) { colorID = id; }
  void SetDepthID(ID id) { depthID = id; }

private:
  GLuint framebufferID;
  FramebufferDesc desc;

  // In order to use the bound textures for this framebuffer, we store them as objects in the
  // GLDevice's object cache. This assigns these pointers to an ID. Therefore, even though the
  // pointers are stored here, these textures are considered to be owned by the device, and are not
  // deleted by the framebuffer;
  ID colorID, depthID;
  GLTexture2D* colorAttachment;
  GLTexture2D* depthStencilAttachment;
};

} // namespace Vision
