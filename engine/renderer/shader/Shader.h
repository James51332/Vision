#pragma once

#include <string>
#include <vector>

namespace Vision
{

enum class ShaderStage
{
  Invalid,
  Vertex,
  Pixel,
  Compute,
  Domain,
  Hull,
  Geometry
};

struct ShaderSource
{
  ShaderStage Stage;
  std::string Name;
  std::string Source;
};

// Shaders no longer exist as objects in the system. Now, the API that the
// user has access to is SPIRV, which is generated entirely separately from the 
// Renderer, via a ShaderCompiler. Render objects accept ShaderSPIRV in their
// descriptors.
struct ShaderSPIRV
{
  ShaderStage Stage;
  std::string Name;
  std::vector<uint32_t> SPIRV;
};

}