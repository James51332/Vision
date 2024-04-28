#include "GLContext.h"

namespace Vision
{

GLContext::GLContext(SDL_Window* wind)
  : window(wind), context(nullptr)
{
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  context = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, context);

  device = new GLDevice();
}

GLContext::~GLContext()
{
  delete device;
}

RenderDevice* GLContext::GetRenderDevice()
{
  return device;
}

void GLContext::Present()
{
  SDL_GL_SwapWindow(window);
}

}