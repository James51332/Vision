#include "App.h"

#include "Input.h"
#include "Macros.h"

namespace Vision
{

App* App::appInstance = nullptr;

App::App(const std::string& name)
  : title(name)
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
  window->Show();

  float lastTime = 0;
  while (running)
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
  }
}

void App::Init()
{
  WindowDesc desc;
  desc.Title = title;

  // Use Metal on macOS
#ifdef VISION_MACOS
  desc.API = RenderAPI::Metal;
#endif

  window = new Window(desc);
  renderContext = window->GetRenderContext();
  displayScale = renderContext->GetDisplayScale();

  // Initialize Input System
  Input::Init();

  // Initialize Rendering Objects
  renderDevice = renderContext->GetRenderDevice();
  renderer = new Renderer(displayWidth, displayHeight, displayScale);
  renderer2D = new Renderer2D(renderDevice, displayWidth, displayHeight, displayScale);
  uiRenderer = new ImGuiRenderer(renderDevice, displayWidth, displayHeight, displayScale);
}

void App::Shutdown()
{
  delete renderer2D;
  delete uiRenderer;
  delete renderer;
  delete window;
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
      case SDL_EVENT_WINDOW_MAXIMIZED:
      {
        // FIX: SDL sends a size of zero after a fullscreen on macOS
        if (event.window.data1 <= 0 || event.window.data2 <= 0)
          break;

        displayWidth = static_cast<float>(event.window.data1);
        displayHeight = static_cast<float>(event.window.data2);
        
        renderContext->Resize(displayWidth, displayHeight);
        renderer->Resize(displayWidth, displayHeight);
        renderer2D->Resize(displayWidth, displayHeight);
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
