#include "ShaderCompiler.h"

#include <iostream>
#include <sstream>
#include <SDL.h>

namespace Vision
{

static const char* StringFromShaderStage(ShaderStage type)
{
  switch (type)
  {
    case ShaderStage::Vertex: return "vertex";
    case ShaderStage::Pixel: return "pixel";
    case ShaderStage::Domain: return "domain";
    case ShaderStage::Hull: return "hull";
    case ShaderStage::Geometry: return "geometry";
    default:
      return "unknown";
  }
}

static ShaderStage ShaderStageFromString(const std::string &type)
{
  if (type == "vertex")
    return ShaderStage::Vertex;
  if (type == "fragment" || type == "pixel")
    return ShaderStage::Pixel;
  if (type == "tesselation_control" || type == "tcs" || type == "hull")
    return ShaderStage::Hull;
  if (type == "tesselation_evaluation" || type == "tes" || type == "domain")
    return ShaderStage::Domain;
  if (type == "geometry")
    return ShaderStage::Geometry;

  SDL_assert(false);
  return ShaderStage::Vertex;
}

void ShaderCompiler::GenerateStageMap(ShaderDesc& desc)
{
  // don't override existing stage map if manually enabled.
  if (desc.loadFromStageMap) return; 

  // read the file from disc.
  SDL_RWops *shader = SDL_RWFromFile(desc.filePath.c_str(), "r+");
  if (!shader)
  {
    SDL_Log("Failed to open shader: %s", SDL_GetError());
  }

  size_t size = SDL_RWsize(shader);
  std::string buffer(size, ' ');
  SDL_RWread(shader, &buffer[0], size);

  // prepare our stage map
  desc.stageMap.clear();

  // System from TheCherno/Hazel
  const char* typeToken = "#type";
  size_t typeTokenLength = strlen(typeToken);
  std::size_t pos = buffer.find(typeToken, 0);
  while (pos != std::string::npos)
  {
    size_t eol = buffer.find_first_of("\r\n", pos); // End of shader type declaration line
    SDL_assert(eol != std::string::npos);
    size_t begin = pos + typeTokenLength + 1; // Start of shader type name (after "#type " keyword)
    std::string type = buffer.substr(begin, eol - begin);

    size_t nextLinePos = buffer.find_first_not_of("\r\n", eol); // Start of shader code after shader type declaration line
    SDL_assert(nextLinePos != std::string::npos);
    pos = buffer.find(typeToken, nextLinePos); // Start of next shader type declaration line

    desc.stageMap[ShaderStageFromString(type)] = (pos == std::string::npos) ? buffer.substr(nextLinePos) : buffer.substr(nextLinePos, pos - nextLinePos);
  }

  // ensure we have a vertex and fragment shader
  SDL_assert(desc.stageMap[ShaderStage::Vertex].size() != 0);
  SDL_assert(desc.stageMap[ShaderStage::Pixel].size() != 0);

  // enable the stage map flag.
  desc.loadFromStageMap = true;
}

}