#include "GLContext.h"

#include <iostream>

namespace Vision
{

GLContext::GLContext(SDL_Window* wind)
  : window(wind), context(nullptr)
{
  // Set the attributes desired for OpenGL rendering
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);

  // On macOS, we must use OpenGL 4.1, but for everything else, we need OpenGL 4.6
#ifdef VISION_MACOS
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#else
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
#endif

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  // Create the GL surface
  context = SDL_GL_CreateContext(window);
  if (!context)
  {
    std::cout << "Failed to Create OpenGL Context!" << std::endl;
    std::cout << SDL_GetError() << std::endl;
    SDL_assert(false);
  }

  SDL_GL_MakeCurrent(window, context);

  // Fetch the size and display scale of the window
  int w, h;
  SDL_GetWindowSizeInPixels(window, &w, &h);

  width = static_cast<float>(w);
  height = static_cast<float>(h);
  displayScale = SDL_GetWindowDisplayScale(window);

  // Instantiate our own GLDevice class handling resource management
  device = new GLDevice(window, width, height);

  // Log the renderer info
  std::cout << "Created OpenGL Context: " << glGetString(GL_VERSION) << std::endl;
  glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
  glGetIntegerv(GL_MINOR_VERSION, &versionMinor);
}

GLContext::~GLContext()
{
  delete device;
}

void GLContext::Resize(float w, float h)
{
  width = w * displayScale;
  height = h * displayScale; 
  device->UpdateSize(width, height);
}

RenderDevice* GLContext::GetRenderDevice()
{
  return device;
}

}