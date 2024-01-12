#include "App.h"

#include "Input.h"
#include "renderer/Mesh.h"

#include <imgui.h>
#include <glm/gtc/random.hpp>

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
    float time = static_cast<float>(SDL_GetTicks()) / 1000.0f;
    if (lastTime != 0)
      timestep = time - lastTime;
    else
      timestep = 0.167f;
    lastTime = time;

    // Process Events
    Input::Update();
    ProcessEvents();

    // Update Camera Controller
    m_PerspectiveCamera->Update(timestep);

    // Render
    glClearColor(0.776f, 0.998f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_Renderer->Begin(m_PerspectiveCamera);
    static int waves = 100;
    m_WaterShader->Use();
    m_WaterShader->UploadUniformInt(waves, "u_Waves");
    m_Renderer->DrawMesh(m_Mesh, m_WaterShader);
    m_Renderer->End();

    // m_UIRenderer->Begin();
    // ImGui::Begin("Waves");
    // ImGui::DragInt("Num Waves", &waves, 1.0, 1, 100);
    // ImGui::End();
    // m_UIRenderer->End();

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

  // Load OpenGL function pointers
  gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

  // Initialize the renderer
  float displayScale = SDL_GetWindowDisplayScale(m_Window);
  m_Renderer = new Renderer(static_cast<float>(w), static_cast<float>(h), displayScale);
  m_UIRenderer = new ImGuiRenderer(static_cast<float>(w), static_cast<float>(h), displayScale);
  m_PerspectiveCamera = new PerspectiveCamera(static_cast<float>(w), static_cast<float>(h));

  GenerateWaves();
  GenerateMesh();
  GenerateShader();
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
    bool processed = UI::ProcessEvent(&event);
    if (processed) continue;

    switch (event.type)
    {
      case SDL_EVENT_WINDOW_RESIZED:
      {
        float width = static_cast<float>(event.window.data1);
        float height = static_cast<float>(event.window.data2);
        m_Renderer->Resize(width, height);
        m_UIRenderer->Resize(width, height);
        m_PerspectiveCamera->SetWindowSize(width, height);
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

void App::GenerateWaves()
{
  srand(time(0));

  float frequency = 5.0f;
  float amplitude = 4.0f; 
  float wavelength = 4.0f; 
  for (int i = 0; i < m_NumWaves; i++)
  {
    Wave& wave = m_Waves[i];
    
    wave.origin = glm::linearRand(glm::vec2(-50.0f, 50.0f), glm::vec2(-50.0f, 50.0f));
    wave.direction = glm::circularRand(1.0f);

    float phase = glm::linearRand(0.0f, static_cast<float>(2.0f * M_PI));
    wave.scale = { amplitude, wavelength, frequency, phase };
    
    wavelength *= 0.9f;
    amplitude *= 0.82f;
  }
  
  BufferDesc desc;
  desc.Size = m_NumWaves * sizeof(Wave);
  desc.Type = GL_UNIFORM_BUFFER;
  desc.Usage = GL_STATIC_DRAW;
  desc.Data = (void*)m_Waves;
  m_WaveBuffer = new Buffer(desc);
}

void App::GenerateMesh()
{
  // Create The Plane Mesh
  constexpr std::size_t planeRes = 512; // I guess this is the most vertices my macbook can handle
  constexpr float planeSize = 10.0f;

  constexpr std::size_t numVertices = planeRes * planeRes;
  constexpr std::size_t numIndices = 6 * (planeRes - 1) * (planeRes - 1); // 6 indices per quad
  {
    MeshDesc desc;
    desc.NumVertices = numVertices;
    desc.NumIndices = numIndices;

    std::vector<MeshVertex> vertices(numVertices);
    float x = -planeSize / 2.0f;
    for (std::size_t i = 0; i < planeRes; i++)
    {
      float z = -planeSize / 2.0f;
      for (std::size_t j = 0; j < planeRes; j++)
      {
        MeshVertex vertex;
        vertex.Position = {x, 0.0f, z};
        vertex.Normal = {0.0f, 1.0f, 0.0f};
        vertex.Color = {0.2f, 0.2f, 0.6f, 1.0f};
        vertex.UV = {static_cast<float>(i) / static_cast<float>(planeRes), static_cast<float>(j) / static_cast<float>(planeRes)};
        vertices[i * planeRes + j] = vertex;

        z += planeSize / static_cast<float>(planeRes);
      }
      x += planeSize / static_cast<float>(planeRes);
    }
    desc.Vertices = vertices;

    std::vector<MeshIndex> indices(numIndices);
    std::size_t index = 0;
    for (std::size_t i = 0; i < planeRes - 1; i++)
    {
      for (std::size_t j = 0; j < planeRes - 1; j++)
      {
        MeshIndex current = static_cast<MeshIndex>(i * planeRes + j);
        MeshIndex right = static_cast<MeshIndex>(current + 1);
        MeshIndex above = static_cast<MeshIndex>(current + planeRes);
        MeshIndex diagonal = static_cast<MeshIndex>(right + planeRes);

        indices[index + 0] = current;
        indices[index + 1] = right;
        indices[index + 2] = diagonal;
        indices[index + 3] = current;
        indices[index + 4] = diagonal;
        indices[index + 5] = above;

        index += 6;
      }
    }
    desc.Indices = indices;

    m_Mesh = new Mesh(desc);
  }
}

void App::GenerateShader()
{
  m_WaterShader = new Shader("resources/waveShader.glsl");
  m_WaterShader->SetUniformBlock(m_WaveBuffer, "WaveProperties", 0);
}

}
