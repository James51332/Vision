#include "MetalCompiler.h"

#include <iostream>
#include <spirv_msl.hpp>

namespace Vision
{
  
MTL::Function* MetalCompiler::Compile(MTL::Device* device, const ShaderSPIRV& shader)
{
  // First perform the decompilation on the SPIRV
  spirv_cross::CompilerMSL decompiler(shader.SPIRV);

  spirv_cross::CompilerMSL::Options options;
  options.enable_decoration_binding = true;
  decompiler.set_msl_options(options);

  std::string msl = decompiler.compile();

  // Next, generate an MTL::Library and use it to create an MTL::Function
  NS::Error* error = nullptr;
  NS::String* code = NS::String::alloc()->init(msl.c_str(), NS::UTF8StringEncoding);
  MTL::Library* library = device->newLibrary(code, nullptr, &error);

  if (error)
  {
    std::cout << "Failed to compile shader: " << shader.Name << std::endl;
    std::cout << error->description()->cString(NS::UTF8StringEncoding) << std::endl;
  }

  NS::String* funcName = NS::String::alloc()->init(shader.Name.c_str(), NS::UTF8StringEncoding);
  MTL::Function* function = library->newFunction(funcName);

  funcName->release();
  library->release();
  code->release();
}

}