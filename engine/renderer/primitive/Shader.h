#pragma once

#include <string>
#include <unordered_map>

namespace Vision
{
  
enum class ShaderStage
{
  Vertex,
  Domain,
  Hull,
  Geometry,
  Pixel,
  Compute
};

static const char* ShaderStageToString(ShaderStage type)
{
  switch (type)
  {
    case ShaderStage::Vertex: return "vertex";
    case ShaderStage::Pixel: return "pixel";
    case ShaderStage::Domain: return "domain";
    case ShaderStage::Hull: return "hull";
    case ShaderStage::Geometry: return "geometry";
    case ShaderStage::Compute: return "compute";
    default:
      return "unknown";
  }
}

/*
This is the shader compilation pipeline. 

file -> stage map -> spirv -> (GLSL/MSL ->) compiler

I'm debating whether or not to include glslang with the vision library.
This really is a key distinction between rendering library and game engine.
If our app supports online shader compilation, we are taking the rendering engine
side; however, if we don't allow for it, but instead include glslang in some external
tool, then our apps are more lightweight, and we take the game engine approach.

Also creating a ShaderDesc api is slightly strange. There are so many different parts of
the pipeline that we may want to support creation of shaders from, at least for now.
Let's create an enum that determines the source of the shader, whether it's a file,
strings, or SPV.
*/

enum class ShaderSource
{
  File,
  StageMap,
  SPIRV
};

// the memory caching on this structure will be awful, but shader compilation shouldn't
// be a part of a run loop or part of the load stage in applications where load time is
// important.
struct ShaderDesc
{
  ShaderSource Source = ShaderSource::File;

  // These are all of the load sources.
  std::string FilePath;
  std::unordered_map<ShaderStage, std::string> StageMap;
  std::unordered_map<ShaderStage, std::vector<uint32_t>> SPIRVMap; 
};

}