#include "GLDevice.h"

#include <iostream>

namespace Vision
{

GLDevice::GLDevice()
{

}

GLDevice::~GLDevice()
{

}

ID GLDevice::CreatePipeline(const PipelineDesc &desc)
{
  if (!shaders.Exists(desc.Shader) || desc.Shader == 0)
  {
    std::cout << "Failed to generate pipeline as shader is invalid" << std::endl;
    return 0;
  }

  GLPipeline* pipeline = new GLPipeline();
  pipeline->Layouts = desc.Layouts;
  pipeline->Shader = shaders.Get(desc.Shader);
  
  ID id = currentID++;
  pipelines.Add(id, pipeline);
  return id;
}

void GLDevice::DestroyPipeline(ID pipeline)
{
  pipelines.Destroy(pipeline);
}

ID GLDevice::CreateShader(const ShaderDesc& desc)
{
  ID id = currentID++;
  GLProgram* shader = new GLProgram(desc.stageMap);
  shaders.Add(id, shader);
  return id;
}

void GLDevice::DestroyShader(ID shader)
{
  shaders.Destroy(shader);
}

static GLenum IndexTypeToGLenum(IndexType type)
{
  switch (type)
  {
  case IndexType::U8:
    return GL_UNSIGNED_BYTE;
  case IndexType::U16:
    return GL_UNSIGNED_SHORT;
  case IndexType::U32:
    return GL_UNSIGNED_INT;
  }
}

static GLenum PrimitiveTypeToGLenum(PrimitiveType type)
{
  switch (type)
  {
  case PrimitiveType::Triangle:
    return GL_TRIANGLES;
  case PrimitiveType::TriangleStrip:
    return GL_TRIANGLE_STRIP;
  case PrimitiveType::Patch:
    return GL_PATCHES;
  }
}

void GLDevice::Submit(const DrawCommand& command)
{
  // bind the shader and upload uniforms
  GLPipeline* pipeline = pipelines.Get(command.Pipeline);
  GLProgram* shader = pipeline->Shader;

  shader->Use();
  GLuint pushConstants = glGetUniformBlockIndex(shader->GetProgram(), "pushConstants");
  glUniformBlockBinding(shader->GetProgram(), pushConstants, 0);

  // bind the textures
  int index = 0;
  for (auto texture : command.Textures)
  {
    texture->Bind(index);
    index++;
  }

  // choose the primitive type and index type
  GLenum primitive = PrimitiveTypeToGLenum(command.Type);
  if (shader->UsesTesselation()) primitive = GL_PATCHES;

  // generate the vertex array using the vertex array cache
  GLVertexArray* vao = vaoCache.Fetch(pipeline, command.VertexBuffers);
  vao->Bind();

  // set the patch size
  glPatchParameteri(GL_PATCH_VERTICES, command.PatchSize);

  // draw
  if (command.IndexBuffer)
  {
    GLenum indexType = IndexTypeToGLenum(command.IndexType);
    command.IndexBuffer->Bind();

    // TODO: Vtx Offsets
    glDrawElements(primitive, command.NumVertices, indexType, nullptr);
  }
  else
  {
    glDrawArrays(primitive, 0, command.NumVertices);
  }
}

}