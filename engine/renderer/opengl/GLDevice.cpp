#include "GLDevice.h"

#include <iostream>
#include <SDL.h>
#include <spirv_glsl.hpp>

#include "GLTypes.h"

#include "renderer/shader/ShaderCompiler.h"

namespace Vision
{

GLDevice::GLDevice(SDL_Window* wind, float w, float h)
  : window(wind), width(w), height(h)
{
  gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

  // Query the version of OpenGL that our context supports
  glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
  glGetIntegerv(GL_MINOR_VERSION, &versionMinor);
}

ID GLDevice::CreateRenderPipeline(const RenderPipelineDesc &desc)
{
  GLPipeline* pipeline = new GLPipeline();
  pipeline->Layouts = desc.Layouts;
  pipeline->Program = new GLProgram(desc.VertexShader, desc.PixelShader, versionMinor < 2 || versionMajor < 4);

  pipeline->DepthTest = desc.DepthTest;
  pipeline->DepthWrite = desc.DepthWrite;
  pipeline->DepthFunc = DepthFuncToGLenum(desc.DepthFunc);

  pipeline->EnableBlend = desc.Blending;
  pipeline->BlendSource = GL_SRC_ALPHA;
  pipeline->BlendDst = GL_ONE_MINUS_SRC_ALPHA;
  
  ID id = currentID++;
  pipelines.Add(id, pipeline);
  return id;
}

ID GLDevice::CreateBuffer(const BufferDesc &desc)
{
  ID id = currentID++;
  GLBuffer* buffer = new GLBuffer(desc);
  buffers.Add(id, buffer);
  return id;
}

void GLDevice::MapBufferData(ID id, void **data, std::size_t size)
{
  GLBuffer* buffer = buffers.Get(id);
  buffer->Bind();
  (*data) = glMapBuffer(buffer->GetType(), GL_READ_ONLY);
}

void GLDevice::FreeBufferData(ID id, void** data)
{
  GLBuffer* buffer = buffers.Get(id);
  buffer->Bind();
  glUnmapBuffer(buffer->GetType());
  (*data) = nullptr;
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

ID GLDevice::CreateFramebuffer(const FramebufferDesc &desc)
{
  ID id = currentID++;
  GLFramebuffer* fb = new GLFramebuffer(desc);
  framebuffers.Add(id, fb);
  return id;
}

ID GLDevice::CreateRenderPass(const RenderPassDesc &desc)
{
  ID id = currentID++;
  RenderPassDesc* obj = new RenderPassDesc(desc);
  renderpasses.Add(id, obj);
  return id;
}

void GLDevice::BeginRenderPass(ID pass)
{
  SDL_assert(commandBufferActive);
  SDL_assert(!activePass);
  SDL_assert(!computePass);

  activePass = pass;
  RenderPassDesc* rp = renderpasses.Get(activePass);
  ID fbID = rp->Framebuffer;

  if (fbID != 0) // don't bind a the default framebuffer.
    framebuffers.Get(fbID)->Bind();

  if (rp->LoadOp == LoadOp::DontCare) return;
  if (rp->LoadOp == LoadOp::Clear)
  {
    glm::vec4& col = rp->ClearColor;
    glClearColor(col.r, col.g, col.b, col.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // TODO: These may want to be controlled separately
  }
}

void GLDevice::EndRenderPass()
{
  SDL_assert(activePass);

  ID fbID = renderpasses.Get(activePass)->Framebuffer;
  if (fbID != 0)
    framebuffers.Get(fbID)->Unbind();
  activePass = 0;

  // render pass StoreOps are pointless in GL.
  glDisable(GL_SCISSOR_TEST);
}

void GLDevice::SetScissorRect(float x, float y, float w, float h)
{
  SDL_assert(activePass);

  if (width <= 0 || height <= 0)
  {
    glScissor(0, 0, width, height);
    glDisable(GL_SCISSOR_TEST);
    return;
  }

  glEnable(GL_SCISSOR_TEST);
  glScissor(x, this->height - (y + h), w, h);
}

void GLDevice::Submit(const DrawCommand& command)
{
  SDL_assert(activePass);

  // bind the shader and upload uniforms
  GLPipeline* pipeline = pipelines.Get(command.RenderPipeline);
  GLProgram* program = pipeline->Program;
  program->Use();

  // setup our GL state
  glDepthMask(pipeline->DepthWrite ? GL_TRUE : GL_FALSE);
  glDepthFunc(pipeline->DepthFunc);
  if (pipeline->DepthTest)
    glEnable(GL_DEPTH_TEST);
  else
    glDisable(GL_DEPTH_TEST);
  
  if (pipeline->EnableBlend)
    glEnable(GL_BLEND);
  else
    glDisable(GL_BLEND);
  glBlendFunc(pipeline->BlendSource, pipeline->BlendDst);

  // choose the primitive type and index type
  GLenum primitive = PrimitiveTypeToGLenum(command.Type);

  // generate the vertex array using the vertex array cache
  GLVertexArray* vao = vaoCache.Fetch(this, command.RenderPipeline, command.VertexBuffers);
  vao->Bind();

  // draw
  if (command.IndexBuffer)
  {
    GLenum indexType = IndexTypeToGLenum(command.IndexType);
    GLBuffer* indexBuffer = buffers.Get(command.IndexBuffer);
    indexBuffer->Bind();

    // TODO: Vtx Offsets
    glDrawElements(primitive, command.NumVertices, indexType, reinterpret_cast<void *>(command.IndexOffset));
  }
  else
  {
    glDrawArrays(primitive, 0, command.NumVertices);
  }
}

void GLDevice::BeginCommandBuffer()
{
  SDL_assert(!commandBufferActive);
  commandBufferActive = true;
}

void GLDevice::SubmitCommandBuffer(bool await)
{
  SDL_assert(!activePass);
  SDL_assert(!computePass);
  SDL_assert(commandBufferActive);
  commandBufferActive = false;

  if (schedulePresent)
  {
    SDL_GL_SwapWindow(window);
    schedulePresent = false;
  }

  if (await)
    glFinish();
}

void GLDevice::SchedulePresentation()
{
  SDL_assert(commandBufferActive);
  SDL_assert(activePass == 0);
  
  schedulePresent = true;
}

void GLDevice::BufferBarrier()
{
  SDL_assert(versionMajor >= 4 && versionMinor >= 3);

  // Until we have a more verbose API, or an intelligent dependency 
  // system, we must block all accesses to buffers in the GPU until
  // the memory becomes visible and accessible.
  glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 
                 | GL_ELEMENT_ARRAY_BARRIER_BIT
                 | GL_UNIFORM_BARRIER_BIT
                 | GL_BUFFER_UPDATE_BARRIER_BIT
                 | GL_SHADER_STORAGE_BARRIER_BIT
                 | GL_QUERY_BUFFER_BARRIER_BIT);
}

void GLDevice::ImageBarrier()
{
  // Memory barriers don't exist in old GL. only use
  SDL_assert(versionMajor >= 4 && versionMinor >= 2);

  glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT
                | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
                | GL_PIXEL_BUFFER_BARRIER_BIT
                | GL_TEXTURE_UPDATE_BARRIER_BIT
                | GL_FRAMEBUFFER_BARRIER_BIT);
}


ID GLDevice::CreateComputePipeline(const ComputePipelineDesc& desc)
{
  // Compute shaders are only suppored on OpenGL 4.3+
  SDL_assert(versionMajor >= 4 && versionMinor >= 3);

  ID id = currentID++;
  GLComputeProgram* program = new GLComputeProgram(desc.ComputeKernels);
  computePrograms.Add(id, program);
  return id;
}

void GLDevice::BeginComputePass() 
{
  SDL_assert(commandBufferActive);
  SDL_assert(!activePass);
  SDL_assert(!computePass);

  // If we assert that all of the conditions are true before beginning,
  // we can simply verify that a valid compute pass is active for each
  // command, instead of checking for a command buffer, etc.
  computePass = true;
}

void GLDevice::EndComputePass() 
{
  SDL_assert(computePass);

  computePass = false;
}

void GLDevice::SetComputeBuffer(ID buffer, std::size_t binding) 
{
  SDL_assert(computePass);
  AttachUniformBuffer(buffer, binding);
}

void GLDevice::SetComputeTexture(ID texture, std::size_t binding) 
{
  SDL_assert(computePass);
  GLTexture2D* tex = textures.Get(texture);
  glBindImageTexture(binding, tex->GetGLID(), 0, GL_FALSE, 0, GL_READ_WRITE, PixelTypeToGLInternalFormat(tex->GetPixelType()));
}

void GLDevice::DispatchCompute(ID pipeline, const std::string& kernel, const glm::ivec3& threads) 
{
  SDL_assert(computePass);

  GLComputeProgram* program = computePrograms.Get(pipeline);
  program->Use(kernel);
  glDispatchCompute(threads.x, threads.y, threads.z);
}


}
