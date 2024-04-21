#include "Framebuffer.h"

#include <SDL.h>

namespace Vision
{

Framebuffer::Framebuffer(float width, float height)
  : m_FramebufferID(0)
{
  Reset(width, height);
}

Framebuffer::~Framebuffer()
{
  delete m_ColorAttachment;
  delete m_DepthStencilAttachment;

  glDeleteFramebuffers(1, &m_FramebufferID);
}

void Framebuffer::Reset(float width, float height)
{
  if (m_FramebufferID)
  {
    delete m_ColorAttachment;
    delete m_DepthStencilAttachment;

    glDeleteFramebuffers(1, &m_FramebufferID);
  }

  m_Width = width;
  m_Height = height;

  glGenFramebuffers(1, &m_FramebufferID);

  m_ColorAttachment = new GLTexture2D(m_Width, m_Height, PixelType::RGBA32, false);
  m_DepthStencilAttachment = new GLTexture2D(m_Width, m_Height, PixelType::Depth32, false);

  glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);
  
  glFramebufferTexture2D(GL_FRAMEBUFFER, 
                         GL_COLOR_ATTACHMENT0, 
                         GL_TEXTURE_2D, 
                         m_ColorAttachment->m_TextureID, 
                         0);

  glFramebufferTexture2D(GL_FRAMEBUFFER,
                         GL_DEPTH_ATTACHMENT,
                         GL_TEXTURE_2D,
                         m_DepthStencilAttachment->m_TextureID,
                         0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    SDL_Log("Failed to complete framebuffer!");

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Bind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);
}

void Framebuffer::Unbind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}
