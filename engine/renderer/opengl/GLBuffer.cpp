#include "GLBuffer.h"

#include <SDL.h>

#include "GLTypes.h"

namespace Vision
{

GLBuffer::GLBuffer(const BufferDesc& desc)
  : m_Type(BufferTypeToGLenum(desc.Type)), m_Usage(BufferUsageToGLenum(desc.Usage)), m_Size(desc.Size)
{
  glGenBuffers(1, &m_Object);
  glBindBuffer(m_Type, m_Object);
  glBufferData(m_Type, m_Size, desc.Data, m_Usage);
}

GLBuffer::~GLBuffer()
{
  glDeleteBuffers(1, &m_Object);
}

void GLBuffer::SetData(void* data, std::size_t size)
{
  SDL_assert(size <= m_Size);

  glBindBuffer(m_Type, m_Object);
  glBufferSubData(m_Type, 0, size, data);
}

void GLBuffer::Resize(std::size_t size)
{
  if (size < m_Size) return; // Don't worry about shrinking

  m_Size = size;
  glBindBuffer(m_Type, m_Object);
  glBufferData(m_Type, m_Size, nullptr, m_Usage);
}

void GLBuffer::Attach(std::size_t block)
{
  glBindBufferBase(m_Type, block, m_Object);
}

void GLBuffer::Bind()
{
  glBindBuffer(m_Type, m_Object);
}

}