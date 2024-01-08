#include "App.h"

#include "Input.h"

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

  float lastTime = 0;

  m_Running = true;
  while (m_Running)
  {
    // Calculate Timestep
    float timestep;
    float time = SDL_GetTicks() / 1000;
    if (lastTime != 0)
      timestep = time - lastTime;
    else
      timestep = 0.16f;
    lastTime = time;

    // Process Events
    Input::Update();
    ProcessEvents();
    
    // Update Camera
    m_Camera->Update(timestep);

    // Render
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_Renderer->BeginFrame(m_Camera, 20.0f);
    m_Renderer->DrawPoint({20.0f, 20.0f}, glm::vec4(1.0f), 5.0);
    m_Renderer->EndFrame();

    SDL_GL_SwapWindow(m_Window);
  }

  Shutdown();
}

void App::Init()
{
  static constexpr int w = 1280;
  static constexpr int h = 720;

  m_Window = SDL_CreateWindow("Vision", w, h, SDL_WINDOW_RESIZABLE 
                                                 | SDL_WINDOW_OPENGL 
                                                 | SDL_WINDOW_HIGH_PIXEL_DENSITY);

  // Get our OpenGL surface to draw on
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  m_Context = SDL_GL_CreateContext(m_Window);
  SDL_GL_MakeCurrent(m_Window, m_Context);

  // VSync
  SDL_GL_SetSwapInterval(1);

  // Initialize Input System
  Input::Init();

  // Initialize the renderer
  float displayScale = SDL_GetWindowDisplayScale(m_Window);
  m_Renderer = new Renderer(static_cast<float>(w), static_cast<float>(h), displayScale);
  m_Camera = new Camera(static_cast<float>(w), static_cast<float>(h), 70.0f);
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
      case SDL_EVENT_WINDOW_RESIZED:
      {
        float width = static_cast<float>(event.window.data1);
        float height = static_cast<float>(event.window.data2);
        m_Camera->SetWindowSize(width, height);
        m_Renderer->Resize(width, height);
        break;
      }
      case SDL_EVENT_QUIT:
      {
        Stop();
        break;
      }
      case SDL_EVENT_KEY_DOWN:
      {
        Input::SetKeyDown(event.key.keysym.scancode);
        break;
      }
      case SDL_EVENT_KEY_UP:
      {
        Input::SetKeyUp(event.key.keysym.scancode);
        break;
      }
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
      {
        Input::SetMouseDown(event.button.button);
        break;
      }
      case SDL_EVENT_MOUSE_BUTTON_UP:
      {
        Input::SetMouseUp(event.button.button);
        break;
      }
      case SDL_EVENT_MOUSE_MOTION:
      {
        Input::SetMousePos(event.motion.x, event.motion.y);
        break;
      }
      case SDL_EVENT_MOUSE_WHEEL:
      {
        Input::SetScrollDelta(event.wheel.x, event.wheel.y);
        break;
      }
      default:
        break;
    }
  }
}

}
