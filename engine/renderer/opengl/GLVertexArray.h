#pragma once

#include <glad/glad.h>
#include <vector>
#include <unordered_map>

#include "renderer/primitive/Buffer.h"

#include "GLPipeline.h"

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

// ----- GLVertexArrayCache -----

// This struct hashes the ID of pipelines and buffers, and generates a vao if needed.
// We can check the cache hit rate late, but in a long-running program, I think it will 
// be high enough.
struct GLVertexArrayCache
{
public:
  GLVertexArray* Fetch(GLPipeline* pipeline, std::vector<Buffer*> vbos);

  void Clear();

private:
  std::unordered_map<std::size_t, GLVertexArray*> vaos;
};

}