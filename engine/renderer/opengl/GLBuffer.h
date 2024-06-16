#pragma once

#include <cstddef>
#include <glad/glad.h>

#include "renderer/primitive/Buffer.h"

namespace Vision
{

class GLBuffer
{
  friend class GLProgram;
public:
  GLBuffer(const BufferDesc &desc);
  ~GLBuffer();

  GLuint GetID() const { return m_Object; }
  GLenum GetType() const { return type; }

  void SetData(void *data, std::size_t size);
  void Resize(std::size_t size); // Resizes but doesn't give data to gpu
  void Attach(std::size_t block);

  void Bind();

private:
  GLuint m_Object;
  GLenum m_Usage;
  GLenum type;
  std::size_t m_Size;

  std::string debugName;
};

}