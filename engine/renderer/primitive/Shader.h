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
  Pixel
};

struct ShaderDesc
{
  std::string filePath;
  
  bool loadFromStageMap = false;
  std::unordered_map<ShaderStage, std::string> stageMap;
};

}