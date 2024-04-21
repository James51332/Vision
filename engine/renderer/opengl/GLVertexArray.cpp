#include "GLVertexArray.h"

#include "GLDevice.h"

namespace Vision
{

GLVertexArray::GLVertexArray()
{
  glGenVertexArrays(1, &m_Object);
}

GLVertexArray::~GLVertexArray()
{
  glDeleteVertexArrays(1, &m_Object);
}

void GLVertexArray::Bind()
{
  glBindVertexArray(m_Object);
}

static GLenum GLenumFromShaderDataType(ShaderDataType type)
{
  switch (type)
  {
    case ShaderDataType::Int:
      return GL_INT;
    case ShaderDataType::Float: 
    case ShaderDataType::Float2:
    case ShaderDataType::Float3:
    case ShaderDataType::Float4:
      return GL_FLOAT;
    case ShaderDataType::UByte4:
      return GL_UNSIGNED_BYTE;
    default:
      return GL_FLOAT;
  }
}

void GLVertexArray::AttachBuffer(GLBuffer* buffer, const BufferLayout& layout)
{  
  glBindVertexArray(m_Object);
  buffer->Bind();

  for (auto& element : layout.Elements)
  {
    glVertexAttribPointer(m_CurrentAttrib, 
                          ShaderDataTypeCount(element.Type), 
                          GLenumFromShaderDataType(element.Type), 
                          element.Normalized, 
                          layout.Stride, 
                          (void*)element.Offset);
    glVertexAttribDivisor(m_CurrentAttrib, element.InstanceDivisor);
    glEnableVertexAttribArray(m_CurrentAttrib);

    m_CurrentAttrib++;
  }
}

// ----- GLVertexArrayCache ------

// https://github.com/DiligentGraphics/DiligentCore/blob/master/Common/interface/HashUtils.hpp#L128
// http://www.boost.org/doc/libs/1_35_0/doc/html/hash/combine.html
template <typename T>
void HashCombine(std::size_t &Seed, const T &Val) noexcept
{
  Seed ^= std::hash<T>()(Val) + 0x9e3779b9 + (Seed << 6) + (Seed >> 2);
}

// template <typename FirstArgType, typename... RestArgsType>
// void HashCombine(std::size_t &Seed, const FirstArgType &FirstArg, const RestArgsType &...RestArgs) noexcept
// {
//   HashCombine(Seed, FirstArg);
//   HashCombine(Seed, RestArgs...); // recursive call using pack expansion syntax
// }

// template <typename FirstArgType, typename... RestArgsType>
// std::size_t ComputeHash(const FirstArgType &FirstArg, const RestArgsType &...RestArgs) noexcept
// {
//   std::size_t Seed = 0;
//   HashCombine(Seed, FirstArg, RestArgs...);
//   return Seed;
// }

// we'll profile this later. just want to ensure we don't have 
// collisions because we aren't hashing the right values.
GLVertexArray* GLVertexArrayCache::Fetch(GLDevice* device, ID pipeline, std::vector<ID> vbos)
{
  // TODO: We really should be hashing the IDs of our pipelines.
  std::size_t hash = 0;
  HashCombine(hash, pipeline);

  GLPipeline* pipeObj = device->GetPipeline(pipeline);

  for (auto layout : pipeObj->Layouts)
  {
    HashCombine(hash, layout.Stride);
    for (auto element : layout.Elements)
    {
      HashCombine(hash, element.InstanceDivisor);
      HashCombine(hash, element.Normalized);
      HashCombine(hash, element.Offset);
      HashCombine(hash, element.Size);
      HashCombine(hash, element.Type);
    }
  }

  for (auto buffer : vbos)
  {
    HashCombine(hash, buffer);
  }

  auto vao = vaos.find(hash);
  if (vao != vaos.end())
  {
    return vao->second;
  }
  else
  {
    GLVertexArray* vao = new GLVertexArray();
    int layoutNum = 0;
    for (auto buffer : vbos)
    {
      vao->AttachBuffer(device->GetBuffer(buffer), pipeObj->Layouts[layoutNum]);
      layoutNum++;
    }

    vaos.emplace(hash, vao);
    return vao;
  }
}

void GLVertexArrayCache::Clear()
{
  vaos.clear();
}


}

