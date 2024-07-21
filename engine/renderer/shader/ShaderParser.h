#pragma once

#include <string>
#include <vector>

#include "Shader.h"

namespace Vision
{

// The idea with this is that I want to continue to use GLSL for now,
// But I want support for multiple programs within a file, beyond just
// one VS and FS. We should also support multiple compute kernels. We'll
// do this by having #section type(vertex) name(vertex)
class ShaderParser
{
public:
  std::vector<ShaderSource> ParseFile(const std::string& file);
};

}