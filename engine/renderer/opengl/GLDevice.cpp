#include "GLDevice.h"

#include <iostream>
#include <SDL.h>
#include <spirv_glsl.hpp>

#include "GLTypes.h"

#include "renderer/ShaderCompiler.h"

namespace Vision
{

GLDevice::GLDevice()
{
  gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
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
  std::unordered_map<GLuint, std::string> samplerBindings; 
  std::unordered_map<GLuint, std::string> uniformBindings;

  // compiler our shader
  if (desc.Source == ShaderSource::File)
  {
    ShaderCompiler compiler;
    compiler.GenerateStageMap(desc);
    compiler.GenerateSPIRVMap(desc);
    desc.Source = ShaderSource::SPIRV;
  }

  // if we manully give the shader code, we won't run the compiler (hack for renderers)
  if (desc.Source == ShaderSource::StageMap)
  {
    shader = new GLProgram(desc.StageMap);
  } 
  else if (desc.Source == ShaderSource::SPIRV)
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
      
      for (auto sampler : res.separate_samplers)
      {
        auto binding = decompiler.get_decoration(sampler.id, spv::DecorationBinding);
        std::string name = sampler.name;
        samplerBindings.emplace(binding, name);
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

    // finally we can do the reflection stuff
    for (auto pair : samplerBindings)
    {
      shader->UploadUniformInt(pair.first, pair.second.c_str());
    }

    for (auto pair : uniformBindings)
    {
      shader->SetUniformBlock(pair.second.c_str(), pair.first);
    }
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
  SDL_assert(activePass == 0);

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
  ID fbID = renderpasses.Get(activePass)->Framebuffer;
  if (fbID != 0)
    framebuffers.Get(fbID)->Unbind();
  activePass = 0;

  // render pass StoreOps are pointless in GL.
}

void GLDevice::SetScissorRect(float x, float y, float width, float height)
{
  if (width <= 0 || height <= 0)
  {
    glDisable(GL_SCISSOR_TEST);
    return;
  }

  glEnable(GL_SCISSOR_TEST);
  glScissor(x, y, width, height);
}

void GLDevice::Submit(const DrawCommand& command)
{
  SDL_assert(activePass != 0);

  // bind the shader and upload uniforms
  GLPipeline* pipeline = pipelines.Get(command.Pipeline);
  GLProgram* shader = pipeline->Shader;

  shader->Use();

  // HACK: we shouldn't do this forever, just getting it working
  shader->SetUniformBlock("pushConstants", 0);

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