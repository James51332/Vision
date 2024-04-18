#pragma once

#include <glad/glad.h>

#include "../Buffer.h"

namespace Vision
{

class GLVertexArray
{
public:
  GLVertexArray();
  ~GLVertexArray();

  void Bind();

  // Buffers are attached in the shader in order of this call 
  void AttachBuffer(Buffer* buffer);

private:
  GLuint m_Object;
  std::size_t m_CurrentAttrib = 0;
};

}