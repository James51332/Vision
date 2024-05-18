#include "MetalShader.h"

#include <iostream>

namespace Vision
{

MetalShader::MetalShader(MTL::Device *device, const std::unordered_map<ShaderStage, std::string> &shaders)
{
  // allocate a compiler options object that we'll use for all of the shaders
  MTL::CompileOptions* options = MTL::CompileOptions::alloc()->init();

  for (auto pair : shaders)
  {
    ShaderStage stage = pair.first;
    std::string source = pair.second;

    // TODO: We may need other forms of encoding later (e.g. wide-chars)
    NS::String* string = NS::String::alloc()->init(source.c_str(), NS::UTF8StringEncoding); 

    // construct a library for each shader stage
    NS::Error* error = nullptr;
    MTL::Library* library = device->newLibrary(string, options, &error);

    if (error)
    {
      std::cout << "Failed to compile shader stage: " << ShaderStageToString(stage) << std::endl;
      std::cout << error->description()->cString(NS::UTF8StringEncoding) << std::endl;
    }

    // extract the main function from the library
    NS::String* funcName = NS::String::alloc()->init("main0", NS::UTF8StringEncoding);
    MTL::Function* func = library->newFunction(funcName);

    shaderFunctions[stage] = func;

    library->release();
  }

  options->release();
}

MetalShader::~MetalShader()
{
  for (auto pair : shaderFunctions)
  {
    pair.second->release(); // release the MTLFunction
  }
}

}