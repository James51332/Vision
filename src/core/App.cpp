#include "App.h"

namespace Vision
{

App::App()
{

}

App::~App()
{

}

void App::Run()
{
  Init();

  m_Running = true;
  while (m_Running)
  {
    ProcessEvents();
  }

  Shutdown();
}

void App::Init()
{
  m_Window = SDL_CreateWindow("Vision", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
}

void App::Shutdown()
{
  SDL_DestroyWindow(m_Window);

  m_Window = nullptr;
}

void App::ProcessEvents()
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
      case SDL_EVENT_QUIT:
      {
        Stop();
        break;
      }
    }
  }
}

}
