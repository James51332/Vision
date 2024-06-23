#include "GLContext.h"

namespace Vision
{

GLContext::GLContext(SDL_Window* wind)
  : window(wind), context(nullptr)
{
  // Set the attributes desired for OpenGL rendering
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  // Create the GL surface
  context = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, context);

  // Fetch the size and display scale of the window
  int w, h;
  SDL_GetWindowSizeInPixels(window, &w, &h);

  width = static_cast<float>(w);
  height = static_cast<float>(h);
  displayScale = SDL_GetWindowDisplayScale(window);

  // Instantiate our own GLDevice class handling resource management
  device = new GLDevice(window, width, height);
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