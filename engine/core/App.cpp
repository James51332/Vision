#include "App.h"

#include "Input.h"

#include "renderer/opengl/GLDevice.h"

namespace Vision
{

App* App::appInstance = nullptr;

App::App(const std::string& title)
  : m_Title(title)
{
  SDL_assert(!appInstance);
  appInstance = this;

  Init();
}

App::~App()
{
  Shutdown();
}

void App::Run()
{
  // Show Window
  SDL_ShowWindow(m_Window);

  float lastTime = 0;
  m_Running = true;
  while (m_Running)
  {
    // Calculate Timestep
    float timestep;
    float time = static_cast<float>(SDL_GetTicks()) / 1000.0f;
    if (lastTime != 0)
      timestep = time - lastTime;
    else
      timestep = 0.167f;
    lastTime = time;

    // Process Events
    Input::Update();
    ProcessEvents();

    // Update App
    OnUpdate(timestep);

    SDL_GL_SwapWindow(m_Window);
  }
}

void App::Init()
{
  m_Window = SDL_CreateWindow(m_Title.c_str(), displayWidth, displayHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN);
  m_DisplayScale = SDL_GetWindowDisplayScale(m_Window);

  // Get our OpenGL surface to draw on (we're gonna move this a to context to remove tie to OpenGL)
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

  // Initialize
  renderDevice = RenderDevice::Create(RenderAPI::OpenGL);
  renderer = new Renderer(displayWidth, displayHeight, m_DisplayScale);
  //renderer2D = new Renderer2D(displayWidth, displayHeight, m_DisplayScale);
  uiRenderer = new ImGuiRenderer(displayWidth, displayHeight, m_DisplayScale);
}

void App::Shutdown()
{
  delete renderer;
 // delete renderer2D;
  delete uiRenderer;
  delete renderDevice;

  SDL_DestroyWindow(m_Window);

  m_Window = nullptr;
}

void App::ProcessEvents()
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    bool processed = UI::ProcessEvent(&event);
    if (processed) continue;

    switch (event.type)
    {
      case SDL_EVENT_WINDOW_RESIZED:
      {
        displayWidth = static_cast<float>(event.window.data1);
        displayHeight = static_cast<float>(event.window.data2);

        renderer->Resize(displayWidth, displayHeight);
        //renderer2D->Resize(displayWidth, displayHeight);
        uiRenderer->Resize(displayWidth, displayHeight);

        OnResize(displayWidth, displayHeight);
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
