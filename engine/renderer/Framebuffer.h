#pragma once

#include <glad/glad.h>

#include "Texture.h"

namespace Vision
{

class Framebuffer
{
public:
  Framebuffer(float width, float height);
  ~Framebuffer();

  void Reset(float width, float height);

  Texture2D* GetColorAttachment() { return m_ColorAttachment; }
  Texture2D* GetDepthAttachment() { return m_DepthStencilAttachment; }

  // If a framebuffer is bound, all rendering will occur on that framebuffer
  void Bind();
  void Unbind();

private:
  GLuint m_FramebufferID;
  float m_Width, m_Height;

  Texture2D* m_ColorAttachment;
  Texture2D* m_DepthStencilAttachment;
};

}