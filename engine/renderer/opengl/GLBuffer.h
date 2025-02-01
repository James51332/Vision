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
  GLBuffer(const BufferDesc& desc);
  ~GLBuffer();

  GLuint GetID() const { return m_Object; }
  GLenum GetType() const { return type; }
  const BufferLayout& GetLayout() const { return m_Layout; }

  void SetLayout(const BufferLayout& layout) { m_Layout = layout; }
  void SetData(void* data, std::size_t size, std::size_t offset);
  void Resize(std::size_t size); // Resizes but doesn't give data to gpu
  void Attach(std::size_t block, std::size_t offset, std::size_t size);

  void Bind();

private:
  GLuint m_Object;
  GLenum m_Usage;
  GLenum type;
  std::size_t m_Size;
  BufferLayout m_Layout;

  std::string debugName;
};

} // namespace Vision