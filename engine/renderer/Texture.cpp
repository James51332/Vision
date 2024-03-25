#include "Texture.h"

#include <SDL.h>

namespace Vision
{

Texture2D::Texture2D(float width, float height, PixelType pixelType, bool writeOnly)
  : m_PixelType(pixelType), m_TextureID(0), m_Renderbuffer(writeOnly)
{
  Resize(width, height);
}

Texture2D::~Texture2D()
{
  if (!m_Renderbuffer)
    glDeleteTextures(1, &m_TextureID);
  else
    glDeleteRenderbuffers(1, &m_TextureID);
}

static GLenum PixelTypeToGLInternalFormat(PixelType type)
{
  switch (type)
  {
    case PixelType::RGBA32: return GL_RGBA8;
    case PixelType::Depth32: return GL_DEPTH_COMPONENT32;
    case PixelType::Depth24Stencil8: return GL_DEPTH24_STENCIL8;
  }
}

static GLenum PixelTypeTOGLFormat(PixelType type)
{
  switch (type)
  {
    case PixelType::RGBA32: return GL_RGBA;
    case PixelType::Depth32: return GL_DEPTH_COMPONENT;
    case PixelType::Depth24Stencil8: return GL_DEPTH_STENCIL;
  }
}

void Texture2D::Resize(float width, float height)
{
  if (m_TextureID)
  {
    if (!m_Renderbuffer)
      glDeleteTextures(1, &m_TextureID);
    else
      glDeleteRenderbuffers(1, &m_TextureID);
  }

  m_Width = width;
  m_Height = height;
  
  if (!m_Renderbuffer)
  {
    glGenTextures(1, &m_TextureID);
    
    glBindTexture(GL_TEXTURE_2D, m_TextureID);
    
    glTexImage2D(GL_TEXTURE_2D, 
                 0, 
                 PixelTypeToGLInternalFormat(m_PixelType),
                 static_cast<GLsizei>(m_Width),
                 static_cast<GLsizei>(m_Height),
                 0,
                 PixelTypeTOGLFormat(m_PixelType),
                 GL_UNSIGNED_BYTE,
                 nullptr);

    // TODO: Expose these parameters to the API
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //glBindTexture(GL_TEXTURE_2D, 0);
  }
  else
  {
    glGenRenderbuffers(1, &m_TextureID);
    glBindRenderbuffer(GL_RENDERBUFFER, m_TextureID);

    glRenderbufferStorage(GL_RENDERBUFFER, 
                          PixelTypeToGLInternalFormat(m_PixelType),
                          static_cast<GLsizei>(m_Width),
                          static_cast<GLsizei>(m_Height));

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
  }
}

void Texture2D::SetData(uint8_t* data)
{
  if (!m_Renderbuffer)
  {
    glBindTexture(GL_TEXTURE_2D, m_TextureID);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    static_cast<GLsizei>(m_Width),
                    static_cast<GLsizei>(m_Height),
                    PixelTypeTOGLFormat(m_PixelType),
                    GL_UNSIGNED_BYTE,
                    static_cast<void*>(data));

    glBindTexture(GL_TEXTURE0, 0);
  }
  else
  {
    // Cannot write to a renderbuffer
    SDL_assert(false);
  }
}

}