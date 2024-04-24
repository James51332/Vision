#include "GLDevice.h"

#include <iostream>

#include "GLTypes.h"

namespace Vision
{

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

ID GLDevice::CreateShader(const ShaderDesc& desc)
{
  ID id = currentID++;
  GLProgram* shader = new GLProgram(desc.StageMap);
  shaders.Add(id, shader);
  return id;
}

ID GLDevice::CreateBuffer(const BufferDesc &desc)
{
  ID id = currentID++;
  GLBuffer* buffer = new GLBuffer(desc);
  buffers.Add(id, buffer);
  return id;
}

ID GLDevice::CreateTexture2D(const Texture2DDesc &desc)
{
  ID id = currentID++;
  GLTexture2D* texture;
  if (desc.LoadFromFile)
    texture = new GLTexture2D(desc.FilePath.c_str());
  else
  {
    texture = new GLTexture2D(desc.Width, desc.Height, desc.PixelType, desc.WriteOnly);
    if (desc.Data)
      texture->SetData(desc.Data);
  }
  
  textures.Add(id, texture);
  return id;
}

ID GLDevice::CreateCubemap(const CubemapDesc &desc)
{
  ID id = currentID++;
  GLCubemap* cubemap = new GLCubemap(desc);
  cubemaps.Add(id, cubemap);
  return id;
}

void GLDevice::Submit(const DrawCommand& command)
{
  // bind the shader and upload uniforms
  GLPipeline* pipeline = pipelines.Get(command.Pipeline);
  GLProgram* shader = pipeline->Shader;

  shader->Use();

  // HACK: we shouldn't do this forever, just getting it working
  shader->SetUniformBlock("pushConstants", 0);

  // bind the textures
  int index = 0;
  for (auto texture : command.Textures)
  {
    textures.Get(texture)->Bind(index);
    index++;
  }

  // choose the primitive type and index type
  GLenum primitive = PrimitiveTypeToGLenum(command.Type);
  if (shader->UsesTesselation()) primitive = GL_PATCHES;

  // generate the vertex array using the vertex array cache
  GLVertexArray* vao = vaoCache.Fetch(this, command.Pipeline, command.VertexBuffers);
  vao->Bind();

  // set the patch size
  glPatchParameteri(GL_PATCH_VERTICES, command.PatchSize);

  // draw
  if (command.IndexBuffer)
  {
    GLenum indexType = IndexTypeToGLenum(command.IndexType);
    GLBuffer* indexBuffer = buffers.Get(command.IndexBuffer);
    indexBuffer->Bind();

    // TODO: Vtx Offsets
    glDrawElements(primitive, command.NumVertices, indexType, nullptr);
  }
  else
  {
    glDrawArrays(primitive, 0, command.NumVertices);
  }
}

}