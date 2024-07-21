#include "GLTexture.h"

#include <SDL.h>
#include <stb_image.h>
#include <iostream>

#include "GLTypes.h"

namespace Vision
{

// ----- GLTexture2D -----

GLTexture2D::GLTexture2D(float width, float height, PixelType pixelType, bool writeOnly)
  : m_PixelType(pixelType), m_TextureID(0), m_Renderbuffer(writeOnly)
{
  Resize(width, height);
}

GLTexture2D::GLTexture2D(const char *filePath)
{
  int w, h, channels;

  // don't support 3 channel images
  stbi_info(filePath, nullptr, nullptr, &channels);
  if (channels == 3) 
    channels = 4;

  unsigned char *data = stbi_load(filePath, &w, &h, nullptr, channels);

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

GLTexture2D::~GLTexture2D()
{
  if (!m_Renderbuffer)
    glDeleteTextures(1, &m_TextureID);
  else
    glDeleteRenderbuffers(1, &m_TextureID);
}

void GLTexture2D::Resize(float width, float height)
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
                 GL_RGBA,
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

void GLTexture2D::SetData(uint8_t *data)
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
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    static_cast<void *>(data));

    glBindTexture(GL_TEXTURE_2D, 0);
  }
  else
  {
    // Cannot write to a renderbuffer
    SDL_assert(false);
  }
}

void GLTexture2D::Bind(uint32_t index)
{
  glActiveTexture(GL_TEXTURE0 + index);
  glBindTexture(GL_TEXTURE_2D, m_TextureID);
}

void GLTexture2D::Unbind()
{
  glBindTexture(GL_TEXTURE_2D, 0);
}

// ----- GLCubemap -----

GLCubemap::GLCubemap(const CubemapDesc &desc)
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
    stbi_info(file.c_str(), nullptr, nullptr, &channels);
    if (channels == 3)
      channels = 4;

    unsigned char *data = stbi_load(file.c_str(), &w, &h, nullptr, channels);
    if (!data)
    {
      std::cout << "Failed to load image:" << file.c_str() << std::endl;
      std::cout << stbi_failure_reason() << std::endl;
      return;
    }

    PixelType pixelType = ChannelsToPixelType(channels);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + side,
                 0,
                 PixelTypeToGLInternalFormat(pixelType),
                 w,
                 h,
                 0,
                 GL_RGBA, // all textures passed as uint8_t*
                 GL_UNSIGNED_BYTE,
                 data);
    side++;
    stbi_image_free(data);
  }

  // set the mag/min params
  // TODO: expose to API
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

GLCubemap::~GLCubemap()
{
  glDeleteTextures(1, &m_CubemapID);
}

void GLCubemap::Bind(uint32_t index)
{
  glActiveTexture(GL_TEXTURE0 + index);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapID);
}

void GLCubemap::Unbind()
{
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

}
