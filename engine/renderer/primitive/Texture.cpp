#include "Texture.h"

#include <SDL.h>
#include <stb_image.h>
#include <iostream>

namespace Vision
{

Texture2D::Texture2D(float width, float height, PixelType pixelType, bool writeOnly)
  : m_PixelType(pixelType), m_TextureID(0), m_Renderbuffer(writeOnly)
{
  Resize(width, height);
}

static PixelType ChannelsToPixelType(int channels)
{
  switch (channels)
  {
    case 1: return PixelType::R8;
    case 2: return PixelType::RG16;
    case 3: return PixelType::RGB24;
    case 4: 
    default:
      return PixelType::RGBA32;
  }
}

Texture2D::Texture2D(const char* filePath)
{
  int w, h, channels;
  unsigned char* data = stbi_load(filePath, &w, &h, &channels, 0);

  if (!data)
  {
    std::cout << "Failed to load image:" << std::endl;
    std::cout << stbi_failure_reason() << std::endl;
  }

  // create our image
  m_PixelType = ChannelsToPixelType(channels);
  Resize(w, h);
  SetData(data);

  // generate mipmaps
  glBindTexture(GL_TEXTURE_2D, m_TextureID);
  glGenerateMipmap(m_TextureID);
  glBindTexture(GL_TEXTURE_2D, 0);

  // free the data from
  stbi_image_free(data);
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
    case PixelType::R8: return GL_R8;
    case PixelType::RG16: return GL_RG8;
    case PixelType::RGB24: return GL_RGB8;
    case PixelType::RGBA32: return GL_RGBA8;
    case PixelType::Depth32: return GL_DEPTH_COMPONENT32;
    case PixelType::Depth24Stencil8: return GL_DEPTH24_STENCIL8;
  }
}

static GLenum PixelTypeTOGLFormat(PixelType type)
{
  switch (type)
  {
    case PixelType::R8: return GL_RED;
    case PixelType::RG16: return GL_RG;
    case PixelType::RGB24: return GL_RGB;
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

    glBindTexture(GL_TEXTURE_2D, 0);
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

void Texture2D::Bind(uint32_t index)
{
  glActiveTexture(GL_TEXTURE0 + index);
  glBindTexture(GL_TEXTURE_2D, m_TextureID);
}

void Texture2D::Unbind()
{
  glBindTexture(GL_TEXTURE_2D, 0);
}

// ----- Cubemap -----

Cubemap::Cubemap(const CubemapDesc& desc)
{
  SDL_assert(desc.Textures.size() == 6);
  
  // generate our texture
  glGenTextures(1, &m_CubemapID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapID);

  // attach the sides to each
  int side = 0;
  for (auto file : desc.Textures)
  {
    int w, h, channels;
    unsigned char *data = stbi_load(file.c_str(), &w, &h, &channels, 0);

    if (!data)
    {
      std::cout << "Failed to load image:" << std::endl;
      std::cout << stbi_failure_reason() << std::endl;
    }

    PixelType pixelType = ChannelsToPixelType(channels);

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, 
                 0, 
                 PixelTypeToGLInternalFormat(pixelType),
                 w,
                 h,
                 0,
                 PixelTypeTOGLFormat(pixelType),
                 GL_UNSIGNED_BYTE,
                 data);

    side++;
    stbi_image_free(data);
  }

  // set the mag/min params
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

Cubemap::~Cubemap()
{
  glDeleteTextures(1, &m_CubemapID);
}

void Cubemap::Bind(uint32_t index)
{
  glActiveTexture(GL_TEXTURE0 + index);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapID);
}

void Cubemap::Unbind()
{
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

}