#pragma once

#include <glad/glad.h>
#include <vector>
#include <string>

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

  float GetWidth() const { return m_Width; }
  float GetHeight() const { return m_Height; }
  PixelType GetPixelType() const { return m_PixelType; }

  void Bind(uint32_t index = 0);
  void Unbind();

private:
  GLuint m_TextureID;

  float m_Width, m_Height;
  PixelType m_PixelType;
  bool m_Renderbuffer;
};

// ----- Cubemaps -----

// right, left, top, bottom, front, back
struct CubemapDesc
{
  std::vector<std::string> Textures;
};

class Cubemap
{
public:
  Cubemap(const CubemapDesc& desc);
  ~Cubemap();

  void Bind(uint32_t index = 0);
  void Unbind();

private:
  GLuint m_CubemapID;
};

}