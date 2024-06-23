project(Vision)

# Add all of the easy submodules
add_subdirectory(vendor/sdl2 EXCLUDE_FROM_ALL)
add_subdirectory(vendor/glad EXCLUDE_FROM_ALL)
add_subdirectory(vendor/glm EXCLUDE_FROM_ALL)
add_subdirectory(vendor/stb EXCLUDE_FROM_ALL)
add_subdirectory(vendor/SPIRV-Cross EXCLUDE_FROM_ALL)
add_subdirectory(vendor/imgui EXCLUDE_FROM_ALL)

# Add metal-cpp on apple
if (APPLE)
  add_subdirectory(vendor/metal-cpp EXCLUDE_FROM_ALL)
endif()

# set the options we want for glslang
foreach(_glslang_option "BUILD_TESTING" "ENABLE_GLSLANG_BINARIES" "ENABLE_SPVREMAPPER" "ENABLE_HLSL" "ENABLE_OPT")
	set(${_glslang_option} OFF CACHE INTERNAL "")
endforeach()
foreach(_glslang_option "SKIP_GLSLANG_INSTALL" "ENABLE_AMD_EXTENSIONS" "ENABLE_NV_EXTENSIONS")
	set(${_glslang_option} ON CACHE INTERNAL "")
endforeach()
add_subdirectory(vendor/glslang)

# Source Files
set(SRC_FILES engine/core/App.cpp
              engine/core/Input.cpp
              engine/core/Window.cpp
              engine/renderer/Camera.cpp
              engine/renderer/Mesh.cpp
              engine/renderer/MeshGenerator.cpp
              engine/renderer/RenderContext.cpp
              engine/renderer/Renderer.cpp
              engine/renderer/Renderer2D.cpp
              engine/renderer/ShaderCompiler.cpp
              engine/renderer/opengl/GLBuffer.cpp
              engine/renderer/opengl/GLContext.cpp
              engine/renderer/opengl/GLDevice.cpp
              engine/renderer/opengl/GLFramebuffer.cpp
              engine/renderer/opengl/GLProgram.cpp
              engine/renderer/opengl/GLTexture.cpp
              engine/renderer/opengl/GLVertexArray.cpp
              engine/ui/ImGuiRenderer.cpp
              engine/ui/UIInput.cpp)

set(APPLE_FILES engine/renderer/metal/MetalBuffer.cpp
                engine/renderer/metal/MetalContext.cpp
                engine/renderer/metal/MetalDevice.cpp
                engine/renderer/metal/MetalFramebuffer.cpp
                engine/renderer/metal/MetalPipeline.cpp
                engine/renderer/metal/MetalRenderPass.cpp
                engine/renderer/metal/MetalShader.cpp
                engine/renderer/metal/MetalTexture.cpp)


# Define the executable for the program
if (APPLE)
  add_library(Vision ${SRC_FILES} ${APPLE_FILES})
else()
  add_library(Vision ${SRC_FILES})
endif()


# Define the include directory for the program
target_include_directories(Vision PUBLIC "engine")

if (APPLE)
  target_link_libraries(Vision PUBLIC
                          "-framework Foundation"
                          "-framework QuartzCore"
                          "-framework Metal")
endif()

# Link to the SDL library
target_link_libraries(Vision
                        PUBLIC
                          SDL3::SDL3
                          glad
                          stb
                          glm::glm
                          ImGui
                          spirv-cross-core
                          spirv-cross-glsl
                          SPIRV
                          glslang-default-resource-limits)

if (APPLE)
target_link_libraries(Vision
                        PUBLIC
                          metal-cpp
                          spirv-cross-msl)
endif()
