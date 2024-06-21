#pragma once

#include <glad/glad.h>
#include <vector>
#include <unordered_map>

#include "GLPipeline.h"
#include "GLBuffer.h"

namespace Vision
{

// forward declare this.
class GLDevice;

class GLVertexArray
{
public:
  GLVertexArray();
  ~GLVertexArray();

  void Bind();

  // Buffers are attached in the shader in order of this call 
  void AttachBuffer(GLBuffer* buffer, const BufferLayout& layout);

private:
  GLuint m_Object;
  std::size_t m_CurrentAttrib = 0;
};

// ----- GLVertexArrayCache -----

// This struct hashes the ID of pipelines and buffers, and generates a vao if needed.
// We can check the cache hit rate late, but in a long-running program, I think it will 
// be high enough.
struct GLVertexArrayCache
{
public:
  GLVertexArray* Fetch(GLDevice* device, ID pipeline, std::vector<ID> vbos);

  void Clear();

private:
  std::unordered_map<std::size_t, GLVertexArray*> vaos;
};

}