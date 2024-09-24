#include "GLFramebuffer.h"

#include <SDL.h>

namespace Vision
{

GLFramebuffer::GLFramebuffer(const FramebufferDesc& desc): framebufferID(0), desc(desc)
{
  Resize(desc.Width, desc.Height);
}

GLFramebuffer::~GLFramebuffer()
{
  delete colorAttachment;
  delete depthStencilAttachment;

  glDeleteFramebuffers(1, &framebufferID);
}

void GLFramebuffer::Resize(float width, float height)
{
  if (framebufferID)
  {
    delete colorAttachment;
    delete depthStencilAttachment;

    glDeleteFramebuffers(1, &framebufferID);
  }

  desc.Width = width;
  desc.Height = height;

  glGenFramebuffers(1, &framebufferID);

  // TODO: Incorporate the formats from the desc
  colorAttachment = new GLTexture2D(desc.Width, desc.Height, PixelType::RGBA8, MinMagFilter::Linear,
                                    MinMagFilter::Linear, EdgeAddressMode::ClampToEdge,
                                    EdgeAddressMode::ClampToEdge);
  depthStencilAttachment = new GLTexture2D(
      desc.Width, desc.Height, PixelType::Depth32Float, MinMagFilter::Linear, MinMagFilter::Linear,
      EdgeAddressMode::ClampToEdge, EdgeAddressMode::ClampToEdge);

  glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         colorAttachment->GetGLID(), 0);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         depthStencilAttachment->GetGLID(), 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    SDL_Log("Failed to complete framebuffer!");

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLFramebuffer::Bind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
}

void GLFramebuffer::Unbind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace Vision