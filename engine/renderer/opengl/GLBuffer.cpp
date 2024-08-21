#include "GLBuffer.h"

#include <SDL.h>

#include "GLTypes.h"

namespace Vision
{

GLBuffer::GLBuffer(const BufferDesc& desc)
  : type(BufferTypeToGLenum(desc.Type)), m_Usage(BufferUsageToGLenum(desc.Usage)), m_Size(desc.Size), debugName(desc.DebugName)
{
  glGenBuffers(1, &m_Object);
  glBindBuffer(type, m_Object);
  glBufferData(type, m_Size, desc.Data, m_Usage);
}

GLBuffer::~GLBuffer()
{
  glDeleteBuffers(1, &m_Object);
}

void GLBuffer::SetData(void* data, std::size_t size)
{
  SDL_assert(size <= m_Size);

  glBindBuffer(type, m_Object);
  glBufferSubData(type, 0, size, data);
}

void GLBuffer::Resize(std::size_t size)
{
  if (size < m_Size) return; // Don't worry about shrinking

  m_Size = size;
  glBindBuffer(type, m_Object);
  glBufferData(type, m_Size, nullptr, m_Usage);
}

void GLBuffer::Attach(std::size_t block, std::size_t offset, std::size_t size)
{
  glBindBufferRange(type, block, m_Object, offset, size == 0 ? m_Size : size);
}

void GLBuffer::Bind()
{
  glBindBuffer(type, m_Object);
}

}