#pragma once

#include <glad/glad.h>

#include "renderer/primitive/Texture.h"

namespace Vision
{

// ----- GLTexture2D -----

// Write only textures are renderbuffers in OpenGL
class GLTexture2D
{
  friend class Framebuffer;
  friend class Renderer2D;

public:
  GLTexture2D(float width, float height, PixelType pixelType, MinMagFilter minFilter, MinMagFilter magFilter, bool renderbuffer = false);
  GLTexture2D(const char *filePath);
  ~GLTexture2D();

  void Resize(float width, float height);
  void SetData(uint8_t *data);
  void SetDataRaw(void *data);

  float GetWidth() const { return m_Width; }
  float GetHeight() const { return m_Height; }
  PixelType GetPixelType() const { return m_PixelType; }
  GLuint GetGLID() const { return m_TextureID; } 

  void Bind(uint32_t index = 0);
  void Unbind();

private:
  GLuint m_TextureID;

  float m_Width, m_Height;
  PixelType m_PixelType;
  MinMagFilter m_MinFilter, m_MagFilter;
  bool m_Renderbuffer = false;
};

// ----- GLCubemap -----

class GLCubemap
{
public:
  GLCubemap(const CubemapDesc &desc);
  ~GLCubemap();

  void Bind(uint32_t index = 0);
  void Unbind();

private:
  GLuint m_CubemapID;
};
}