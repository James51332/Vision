#include "VertexArray.h"

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

void GLVertexArray::AttachBuffer(Buffer* buffer)
{
  const BufferLayout& layout = buffer->GetLayout();
  
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

}

