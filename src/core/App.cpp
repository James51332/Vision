#include "App.h"

#include "Input.h"
#include "renderer/Mesh.h"

#include <imgui.h>

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
    m_Renderer->DrawMesh(m_Mesh, m_WaterShader);
    m_Renderer->End();

    m_UIRenderer->Begin();
    ImGui::ShowDemoWindow();
    m_UIRenderer->End();

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

  // Create The Plane Mesh
  constexpr std::size_t planeRes = 150;
  constexpr float planeSize = 40.0f;

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
        vertex.Position = {x, -1.0f, z};
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

  // Create Shader
  {
  const char *vertex = R"(
  #version 330 core

  layout (location = 0) in vec3 a_Pos;

  out vec3 v_WorldPos;
  out vec3 v_Normal;
  out vec3 v_CamPos;

  uniform mat4 u_ViewProjection;
  uniform float u_Time;
  uniform vec3 u_CameraPos;

  void main()
  {
    float offset = 0.4f * sin(a_Pos.x + u_Time) + 0.5f * sin(a_Pos.z + u_Time);
    vec3 gradientX = vec3(1.0, 0.4f * cos(a_Pos.x + u_Time), 0.0);
    vec3 gradientZ = vec3(0.0, 0.5f * cos(a_Pos.z + u_Time), 1.0);
    v_Normal = cross(gradientZ, gradientX);

    vec3 pos = a_Pos;
    pos.y += offset;

    v_WorldPos = pos;
    v_CamPos = u_CameraPos;

    gl_Position = u_ViewProjection * vec4(pos, 1.0);
  })";

  const char *fragment = R"(
  #version 330 core

  in vec3 v_WorldPos;
  in vec3 v_Normal;
  in vec3 v_CamPos;

  layout (location = 0) out vec4 f_FragColor;

  void main()
  {
    vec3 lightPos = vec3(-2000.0, 2000.0, -2000.0);
    vec3 norm = normalize(v_Normal);
    vec3 lightDir = normalize(lightPos - v_WorldPos);  
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    vec3 diffuse = max(dot(norm, lightDir), 0) * 0.6 * lightColor;
    vec3 ambient = 0.5 * lightColor;

    vec3 viewDir = normalize(v_CamPos - v_WorldPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 4);
    vec3 specular = 0.2f * spec * lightColor;  

    vec3 color = (ambient + diffuse + specular) * vec3(0.2, 0.2, 0.6);
    f_FragColor = vec4(color, 1.0);
  })";

  m_WaterShader = new Shader(vertex, fragment);
  }
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

}
