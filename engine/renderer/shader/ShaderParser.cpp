#include "ShaderParser.h"

#include <iostream>
#include <SDL.h>

namespace Vision
{

// Helper Function
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
  if (type == "compute")
    return ShaderStage::Compute;

  SDL_assert(false);
  return ShaderStage::Vertex;
}

std::vector<ShaderSource> ShaderParser::ParseFile(const std::string& filePath)
{
  // Step 1) Load the file from disk
  SDL_RWops *file = SDL_RWFromFile(filePath.c_str(), "r+");
  if (!file)
  {
    std::cout << "Failed to open shader file: " << SDL_GetError() << std::endl;
    return {};
  }

  size_t size = SDL_RWsize(file);
  std::string source(size, ' ');
  SDL_RWread(file, &source[0], size);
  SDL_RWclose(file);

  // Step 2) Iterate over the sections of the string, breaking apart into ShaderSources.
  const char *sectionToken = "#section ";
  const size_t sectionTokenLength = std::strlen(sectionToken);
  std::vector<ShaderSource> sources;
  std::string commonText; // appends common source as we iterate

  std::size_t pos = source.find(sectionToken, 0);
  while (pos != std::string::npos)
  {
    // Step 3) Extract the section decorations
    size_t eol = source.find_first_of("\r\n", pos);
    SDL_assert(eol != std::string::npos); // ensure that we have another line
    size_t begin = pos + sectionTokenLength; // start of decorations
    std::string decorations = source.substr(begin, eol - begin); // substr to get type value

    // Step 4) Parse the section decorations
    std::string name;
    ShaderStage stage;
    bool common = false;
    std::size_t startIndex = decorations.find_first_not_of(' ');
    while (startIndex != std::string::npos) 
    {
      std::size_t endIndex = decorations.find_first_of(' ', startIndex);
      std::string decoration = (endIndex == std::string::npos) ? decorations.substr(startIndex) : decorations.substr(startIndex, endIndex - startIndex);
      
      std::size_t paramIndex = decoration.find_first_of('(');
      std::string decorType = decoration.substr(0, paramIndex);

      // Parse the parameter if one exists
      std::string paramValue;
      if (paramIndex != std::string::npos)
      {  
        paramIndex++; // start after the opening parenthesis
        char paramChar = decoration[paramIndex];
        while (std::isalpha(paramChar))
        {
          paramValue.push_back(paramChar);
          paramChar = decoration[++paramIndex];
        }

        if (paramChar != ')')
        {
          std::cout << "ShaderParser Error: " << filePath << std::endl;
          std::cout << "Decoration parameters may only contain letters and must end with ')'!" << std::endl;
          startIndex = decorations.find_first_not_of(' ', endIndex);
          continue;
        }
      }

      // Handle each parameter type
      if (decorType == "common") 
        common = true;
      else if (decorType == "name")
        name = paramValue;
      else if (decorType == "type")
        stage = ShaderStageFromString(paramValue);
      else
      {
        std::cout << "Unknown Shader Decoration: " << decorType << std::endl;
        return {};
      }

      startIndex = decorations.find_first_not_of(' ', endIndex);
    }

    // Step 5) Substring to get our next section of shader code
    size_t nextLinePos = source.find_first_not_of("\r\n", eol); // Start of shader code after shader type declaration line
    SDL_assert(nextLinePos != std::string::npos);
    pos = source.find(sectionToken, nextLinePos); // Start of next shader type declaration line
    std::string section = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);

    // Step 6) handle common type by adding to common buffer as well as the end of any parsed strings
    if (common)
    {
      commonText.append("\n"); // append a new line so neither original nor new need line break
      commonText.append(section);

      for (auto& source : sources)
      {
        source.Source.append("\n"); // append a new line so neither original nor new need line break
        source.Source.append(section);
      }

      continue; // don't create a new section
    }

    // Step 7) otherwise, create a new shader stage
    std::string combinedSource = commonText;
    combinedSource.append(section);
    sources.push_back({ stage, name, combinedSource });
  }

  // Step 8) Return our parsed shaders
  return sources;
}

}