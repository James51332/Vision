#include "GLDevice.h"

#include <iostream>
#include <SDL.h>
#include <spirv_glsl.hpp>

#include "GLTypes.h"

#include "renderer/ShaderCompiler.h"

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

ID GLDevice::CreateShader(const ShaderDesc& tmp)
{
  ID id = currentID++;
  GLProgram* shader;
  
  ShaderDesc desc = tmp; // we need to modify, but we'll just copy (don't mess w/ user stuff—offline anyways)
  
  // our naive approach to shader reflection.
  struct Sampler
  {
    std::string name;
    int binding;
    int size;
  };

  std::vector<Sampler> samplers;
  std::unordered_map<GLuint, std::string> uniformBindings;

  // compiler our shader
  if (desc.Source == ShaderSource::File)
  {
    ShaderCompiler compiler;
    compiler.GenerateStageMap(desc);
  }

  if (desc.Source == ShaderSource::GLSL)
  {
    ShaderCompiler compiler;
    compiler.GenerateSPIRVMap(desc);
    desc.Source = ShaderSource::SPIRV;
  }

  if (desc.Source == ShaderSource::SPIRV)
  {
    // first we need to decompile
    std::unordered_map<ShaderStage, std::string> stages;
    for (auto pair : desc.SPIRVMap)
    {
      auto stage = pair.first;
      auto& data = pair.second;
      spirv_cross::CompilerGLSL decompiler(data);

      // TODO: We'll consider options which are more suited for modern OpenGL
      // once metal is implemented, and we want compute shaders on other platforms.
      spirv_cross::CompilerGLSL::Options options;
      options.emit_push_constant_as_uniform_buffer = true;
      options.enable_420pack_extension = false;
      options.version = 410;
      decompiler.set_common_options(options);

      std::string glsl = decompiler.compile();
      stages[stage] = glsl;

      // we're also going to handle automatically binding shader resources for now.
      // in the future, we may want a more robust system, but this should get us going.
      spirv_cross::ShaderResources res = decompiler.get_shader_resources();

      for (auto &e : res.sampled_images)
      {
        const spirv_cross::SPIRType &type = decompiler.get_type(e.type_id);

        Sampler s;
        s.name = e.name;
        s.binding = decompiler.get_decoration(e.id, spv::DecorationBinding);

        // handle sampler2D[]
        if (type.array.size())
        {
          s.size = type.array.front();
        }
        else
        {
          s.size = 1;
        }

        samplers.push_back(s);
      }

      for (auto uniform : res.uniform_buffers)
      {
        auto block = decompiler.get_decoration(uniform.id, spv::DecorationBinding);
        std::string name = uniform.name;
        uniformBindings.emplace(block, name);
      }
    }

    // then we need to build
    shader = new GLProgram(stages);
    shader->Use();

    // finally we can do the reflection stuff
    for (auto sampler : samplers)
    {
      if (sampler.size == 1)
        shader->UploadUniformInt(sampler.binding, sampler.name.c_str());
      else
      {
        // ensure that our binding array is {binding, binding + 1, binding + 2, ...}
        std::vector bindings(sampler.size, sampler.binding);
        for (std::size_t i = 0; i < sampler.size; i++) 
          bindings[i] += i;

        shader->UploadUniformIntArray(bindings.data(), sampler.size, sampler.name.c_str());
      }
    }

    for (auto pair : uniformBindings)
    {
      shader->SetUniformBlock(pair.second.c_str(), pair.first);
    }
  }
  else
  {
    SDL_Log("Unsupported Shader Source in OpenGL!"); // atp, all valid source have been handled.
    SDL_assert(false);
    return 0;
  }

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
  GLPipeline* pipeline = pipelines.Get(command.Pipeline);
  GLProgram* shader = pipeline->Shader;
  shader->Use();

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
  if (shader->UsesTesselation())
    glPatchParameteri(GL_PATCH_VERTICES, command.PatchSize);

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

ID GLDevice::CreateComputePipeline(const ComputePipelineDesc& desc)
{
  // Compute shaders are only suppored on OpenGL 4.3+
  SDL_assert(versionMajor >= 4 && versionMinor >= 3);

  ID id = currentID++;
  GLComputeProgram* program = new GLComputeProgram(desc);
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
  BindTexture2D(texture, binding);
}

void GLDevice::DispatchCompute(ID pipeline, const glm::vec3& threads) 
{
  SDL_assert(computePass);

  GLComputeProgram* program = computePrograms.Get(pipeline);
  program->Use();
  glDispatchCompute(threads.x, threads.y, threads.z);
}


}
