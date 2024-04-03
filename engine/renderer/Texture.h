#pragma once

#include <glad/glad.h>

namespace Vision
{

enum class PixelType
{
  R8,
  RG16,
  RGB24,
  RGBA32,
  Depth32,
  Depth24Stencil8,
};

// Write only textures are renderbuffers in OpenGL
class Texture2D
{
  friend class Framebuffer;
  friend class Renderer2D;
public:
  Texture2D(float width, float height, PixelType pixelType, bool renderbuffer = false);
  Texture2D(const char* filePath);
  ~Texture2D();

  void Resize(float width, float height);
  void SetData(uint8_t* data);

  GLuint m_TextureID;
private:

  float m_Width, m_Height;
  PixelType m_PixelType;
  bool m_Renderbuffer;
};

}