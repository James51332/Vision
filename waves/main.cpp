#include "core/App.h"

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <imgui.h>
#include <vector>
#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>

#include "renderer/Camera.h"
#include "renderer/Renderer.h"
#include "renderer/Mesh.h"
#include "renderer/Shader.h"

#include "ui/ImGuiRenderer.h"

struct Wave
{
  glm::vec2 origin;
  glm::vec2 direction;
  glm::vec4 scale; // amplitude, wavelength, angular frequency, phase
};

class Waves : public Vision::App
{
public:
  Waves()
  {
    // Initialize the renderer
    m_Renderer = new Vision::Renderer(m_DisplayWidth, m_DisplayHeight, m_DisplayScale);
    m_UIRenderer = new Vision::ImGuiRenderer(m_DisplayWidth, m_DisplayHeight, m_DisplayScale);
    m_PerspectiveCamera = new Vision::PerspectiveCamera(m_DisplayWidth, m_DisplayHeight);

    GenerateWaves();
    GenerateMesh();
    GenerateShader();
  }

  ~Waves()
  {
    delete m_Mesh;
    delete m_Renderer;
    delete m_UIRenderer;
    delete m_PerspectiveCamera;
  }

  void OnUpdate(float timestep)
  {
    // Update Camera Controller
    m_PerspectiveCamera->Update(timestep);

    // Render
    glClearColor(0.776f, 0.998f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static int waves = 100;
    m_Renderer->Begin(m_PerspectiveCamera);
    {
      m_WaterShader->Use();
      m_WaterShader->UploadUniformInt(waves, "u_Waves");
      m_Renderer->DrawMesh(m_Mesh, m_WaterShader);
    }
    m_Renderer->End();

    m_UIRenderer->Begin();
    {
      ImGui::Begin("Waves");
      ImGui::DragInt("Num Waves", &waves, 1.0, 1, 100);
      ImGui::End();
    }
    m_UIRenderer->End();
  }

  void OnResize()
  {
    m_Renderer->Resize(m_DisplayWidth, m_DisplayHeight);
    m_UIRenderer->Resize(m_DisplayWidth, m_DisplayHeight);
    m_PerspectiveCamera->SetWindowSize(m_DisplayWidth, m_DisplayHeight);
  }

private:
  void GenerateWaves()
  {
    srand(time(0));

    float frequency = 5.0f;
    float amplitude = 4.0f; 
    float wavelength = 10.0f; 
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

    Vision::BufferDesc desc;
    desc.Size = m_NumWaves * sizeof(Wave);
    desc.Type = GL_UNIFORM_BUFFER;
    desc.Usage = GL_STATIC_DRAW;
    desc.Data = (void*)m_Waves;
    m_WaveBuffer = new Vision::Buffer(desc);
  }

  void GenerateMesh()
  {
    // Create The Plane Mesh
    constexpr std::size_t planeRes = 1024; // I guess this is the most vertices my macbook can handle
    constexpr float planeSize = 50.0f;

    constexpr std::size_t numVertices = planeRes * planeRes;
    constexpr std::size_t numIndices = 6 * (planeRes - 1) * (planeRes - 1); // 6 indices per quad
    {
      Vision::MeshDesc desc;
      desc.NumVertices = numVertices;
      desc.NumIndices = numIndices;

      std::vector<Vision::MeshVertex> vertices(numVertices);
      float x = -planeSize / 2.0f;
      for (std::size_t i = 0; i < planeRes; i++)
      {
        float z = -planeSize / 2.0f;
        for (std::size_t j = 0; j < planeRes; j++)
        {
          Vision::MeshVertex vertex;
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

      std::vector<Vision::MeshIndex> indices(numIndices);
      std::size_t index = 0;
      for (std::size_t i = 0; i < planeRes - 1; i++)
      {
        for (std::size_t j = 0; j < planeRes - 1; j++)
        {
          Vision::MeshIndex current = static_cast<Vision::MeshIndex>(i * planeRes + j);
          Vision::MeshIndex right = static_cast<Vision::MeshIndex>(current + 1);
          Vision::MeshIndex above = static_cast<Vision::MeshIndex>(current + planeRes);
          Vision::MeshIndex diagonal = static_cast<Vision::MeshIndex>(right + planeRes);

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

      m_Mesh = new Vision::Mesh(desc);
    }
  }

  void GenerateShader()
  {
    // This code aint good but it get its done for now.
    std::string vert, frag;
    {
      // Read SPIR-V from disk or similar.
      SDL_RWops* file = SDL_RWFromFile("resources/waterVertex.spv", "rb");
      assert(file);
      SDL_RWseek(file, 0, SDL_RW_SEEK_END);
      std::size_t size = SDL_RWtell(file);
      SDL_RWseek(file, 0, SDL_RW_SEEK_SET);
      std::vector<uint32_t> spirv_binary(size / 4, 0);
      SDL_RWread(file, spirv_binary.data(), size);
      spirv_cross::CompilerGLSL glsl(spirv_binary);

      // Set some options.
      spirv_cross::CompilerGLSL::Options options;
      options.version = 410;
      options.es = false;
      options.enable_420pack_extension = false;
      glsl.set_common_options(options);

      // Compile to GLSL, ready to give to GL driver.
      vert = glsl.compile();
    }

    {
      // Read SPIR-V from disk or similar.
      SDL_RWops *file = SDL_RWFromFile("resources/waterFrag.spv", "rb");
      assert(file);
      SDL_RWseek(file, 0, SDL_RW_SEEK_END);
      std::size_t size = SDL_RWtell(file);
      SDL_RWseek(file, 0, SDL_RW_SEEK_SET);
      std::vector<uint32_t> spirv_binary(size / 4, 0);
      SDL_RWread(file, spirv_binary.data(), size);
      spirv_cross::CompilerGLSL glsl(spirv_binary);

      // Set some options.
      spirv_cross::CompilerGLSL::Options options;
      options.version = 410;
      options.es = false;
      options.enable_420pack_extension = false;
      glsl.set_common_options(options);

      // Compile to GLSL, ready to give to GL driver.
      frag = glsl.compile();
    }

    m_WaterShader = new Vision::Shader(vert.c_str(), frag.c_str());
    m_WaterShader->SetUniformBlock(m_WaveBuffer, "WaveProperties", 0); 
  }

private:
  Vision::PerspectiveCamera *m_PerspectiveCamera;
  Vision::Renderer *m_Renderer;
  Vision::ImGuiRenderer *m_UIRenderer;
  Vision::Mesh *m_Mesh;
  Vision::Shader *m_WaterShader;

  constexpr static std::size_t m_NumWaves = 100;
  Wave m_Waves[m_NumWaves];
  Vision::Buffer *m_WaveBuffer;
};

int main()
{
  Waves* waves = new Waves();
  waves->Run();
  delete waves;
}
