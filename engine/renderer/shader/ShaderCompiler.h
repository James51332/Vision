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

  std::vector<ShaderSPIRV> CompileFile(const std::string& filePath, bool canCache = false);
  void CompileFile(const std::string& filePath, std::vector<ShaderSPIRV>& destination,
                   bool canCache = false);
  std::unordered_map<std::string, ShaderSPIRV> CompileFileToMap(const std::string& filePath,
                                                                bool canCache = false);
};

} // namespace Vision