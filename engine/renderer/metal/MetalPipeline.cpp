#include "MetalPipeline.h"

#include <iostream>
#include <SDL.h>

#include <spirv_msl.hpp>

#include "renderer/ShaderCompiler.h"

#include "MetalType.h"

namespace Vision
{

// ----- MetalPipeline -----

MetalPipeline::MetalPipeline(MTL::Device* device, ObjectCache<MetalShader>& shaders, const PipelineDesc& desc)
{
  MTL::RenderPipelineDescriptor *attribs = MTL::RenderPipelineDescriptor::alloc()->init();

  // set the pixel format
  attribs->colorAttachments()->object(0)->setPixelFormat(PixelTypeToMTLPixelFormat(desc.PixelFormat));
  attribs->colorAttachments()->object(0)->setBlendingEnabled(desc.Blending);
  attribs->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
  attribs->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
  attribs->colorAttachments()->object(0)->setAlphaBlendOperation(MTL::BlendOperationAdd);
  attribs->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
  attribs->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
  attribs->colorAttachments()->object(0)->setRgbBlendOperation(MTL::BlendOperationAdd);

  // link the shader
  MetalShader *shader = shaders.Get(desc.Shader);
  SDL_assert(shader);
  attribs->setVertexFunction(shader->GetFunction(ShaderStage::Vertex));
  attribs->setFragmentFunction(shader->GetFunction(ShaderStage::Pixel));

  // set the pipeline layout
  MTL::VertexDescriptor* vtxDesc = MTL::VertexDescriptor::alloc()->init();
  
  // build the free buffer bindings
  auto& usedSlots = shader->GetUniformBufferSlots();
  constexpr std::size_t maxSlot = 30; // this is the last slot in the table
  stageBufferBindings.clear();
  
  for (std::size_t i = 0; i <= maxSlot; i++)
  {
    std::size_t count = std::count(usedSlots.begin(), usedSlots.end(), i);
    if (count == 1) 
      continue;
    else if (count == 0) 
      stageBufferBindings.push_back(i);
    else
      SDL_assert(false);

    if (stageBufferBindings.size() == desc.Layouts.size()) // we have enough slots.
      break;
  }

  int stageBuffer = 0;
  int attrib = 0;
  for (auto layout : desc.Layouts)
  {
    SDL_assert(stageBuffer < stageBufferBindings.size()); // We can't have more stage slots than free slots
    std::size_t layoutIndex = stageBufferBindings[stageBuffer];

    if (layout.Stride == 0)
    {
      stageBuffer++;
      continue;
    }
    
    for (auto elem : layout.Elements)
    {
      vtxDesc->attributes()->object(attrib)->setBufferIndex(layoutIndex);
      vtxDesc->attributes()->object(attrib)->setFormat(ShaderDataTypeToMTLVertexFormat(elem.Type, elem.Normalized));
      vtxDesc->attributes()->object(attrib)->setOffset(elem.Offset);

      attrib++;
    }

    vtxDesc->layouts()->object(layoutIndex)->setStride(layout.Stride);
    stageBuffer++;
  }

  attribs->setVertexDescriptor(vtxDesc);
  
  attribs->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float);

  // build the pipeline
  NS::Error *error = nullptr;
  pipeline = device->newRenderPipelineState(attribs, &error);

  if (error)
  {
    std::cout << "Failed to build render pipeline state!" << std::endl;
    std::cout << error->description()->cString(NS::UTF8StringEncoding) << std::endl;
  }

  vtxDesc->release();
  attribs->release();

  // create the depth state
  MTL::DepthStencilDescriptor* depthDesc = MTL::DepthStencilDescriptor::alloc()->init();  
  depthDesc->setDepthCompareFunction(desc.DepthTest ? DepthFunctionToMTLCompareFunction(desc.DepthFunc) : MTL::CompareFunctionAlways);
  depthDesc->setDepthWriteEnabled(desc.DepthWrite);

  depthState = device->newDepthStencilState(depthDesc);
  depthDesc->release();
}

MetalPipeline::~MetalPipeline()
{
  pipeline->release();
  depthState->release();
}

// ----- MetalComputePipeline -----

MetalComputePipeline::MetalComputePipeline(MTL::Device *device, ComputePipelineDesc &desc)
{
  std::string msl;

  // prepare our shader code
  if (desc.Source == ShaderSource::File)
  {
    ShaderCompiler compiler;
    compiler.LoadSource(desc);
    compiler.GenerateSPIRV(desc);
  }

  if (desc.Source == ShaderSource::GLSL)
  {
    ShaderCompiler compiler;
    compiler.GenerateSPIRV(desc);
  }

  if (desc.Source == ShaderSource::SPIRV)
  {
    spirv_cross::CompilerMSL compiler(std::move(desc.SPIRV));
    
    auto mslOpts = compiler.get_msl_options();
    mslOpts.enable_decoration_binding = true; // tell the compiler to use glsl bindings for buffer indices
    compiler.set_msl_options(mslOpts);

    msl = compiler.compile();
    // std::cout << msl << std::endl;

    // fetch the threadgroup info from GLSL (HACK: There may be a better way to do this)
    auto entries = compiler.get_entry_points_and_stages();
    auto workSize = compiler.get_entry_point(entries.front().name, entries.front().execution_model).workgroup_size;
    workGroupSize = { workSize.x, workSize.y, workSize.z };
  }

  // allocate a compiler options object that we'll use for all of the shaders
  MTL::CompileOptions *options = MTL::CompileOptions::alloc()->init();

  // TODO: We may need other forms of encoding later (e.g. wide-chars)
  NS::String *string = NS::String::alloc()->init(msl.c_str(), NS::UTF8StringEncoding);

  // construct a library for the shader stage
  NS::Error *error = nullptr;
  MTL::Library *library = device->newLibrary(string, options, &error);

  if (error)
  {
    std::cout << "Failed to compile compute shader" << std::endl;
    std::cout << error->description()->cString(NS::UTF8StringEncoding) << std::endl;
  }

  // extract the main function from the library
  NS::String *funcName = NS::String::alloc()->init("main0", NS::UTF8StringEncoding);
  MTL::Function *func = library->newFunction(funcName);

  // build the pipeline
  pipeline = device->newComputePipelineState(func, &error);
  if (error)
  {
    std::cout << "Failed to link compute pipeline" << std::endl;
    std::cout << error->description()->cString(NS::UTF8StringEncoding) << std::endl;
  }

  // free all of our memory.
  library->release();
  string->release();
  options->release();
  func->release();
}

MetalComputePipeline::~MetalComputePipeline()
{
  pipeline->release();
}

}
