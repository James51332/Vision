#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Shader.h"
#include "ShaderParser.h"

namespace Vision
{

class ShaderCompiler
{
public:
  ShaderSPIRV CompileSource(const ShaderSource& shaderSource);

  std::vector<ShaderSPIRV> CompileFile(const std::string& filePath);
  void CompileFile(const std::string& filePath, std::vector<ShaderSPIRV>& destination);
  std::unordered_map<std::string, ShaderSPIRV> CompileFileToMap(const std::string& filePath);
};

} // namespace Vision