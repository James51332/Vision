#pragma once

#include <SDL3/SDL.h>

namespace Vision
{

class App
{
public:
  App();
  ~App();

  void Run();
  void Stop() { m_Running = false; }

private:
  void Init();
  void Shutdown();
  void ProcessEvents();

private:
  // Run Loop
  bool m_Running = false;

  // Platform Data
  SDL_Window* m_Window;
};

}