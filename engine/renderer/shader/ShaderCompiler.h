#pragma once

#include <string>

#include "Shader.h"
#include "ShaderParser.h"

namespace Vision
{

class ShaderCompiler
{
public:
  ShaderSPIRV CompileSource(const ShaderSource& shaderSource);
  std::vector<ShaderSPIRV> CompileFile(const std::string& filePath);
  void CompileFile(const std::string &filePath, std::vector<ShaderSPIRV>& destination);
};

}