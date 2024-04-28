project(Vision)

# Add all of the easy submodules
add_subdirectory(vendor/sdl2 EXCLUDE_FROM_ALL)
add_subdirectory(vendor/glad EXCLUDE_FROM_ALL)
add_subdirectory(vendor/glm EXCLUDE_FROM_ALL)
add_subdirectory(vendor/stb EXCLUDE_FROM_ALL)
add_subdirectory(vendor/SPIRV-Cross EXCLUDE_FROM_ALL)
add_subdirectory(vendor/imgui EXCLUDE_FROM_ALL)
add_subdirectory(vendor/metal-cpp EXCLUDE_FROM_ALL)

# set the options we want for glslang
foreach(_glslang_option "BUILD_TESTING" "ENABLE_GLSLANG_BINARIES" "ENABLE_SPVREMAPPER" "ENABLE_HLSL" "ENABLE_OPT")
	set(${_glslang_option} OFF CACHE INTERNAL "")
endforeach()
foreach(_glslang_option "SKIP_GLSLANG_INSTALL" "ENABLE_AMD_EXTENSIONS" "ENABLE_NV_EXTENSIONS")
	set(${_glslang_option} ON CACHE INTERNAL "")
endforeach()
add_subdirectory(vendor/glslang)

# Source Files
file(GLOB_RECURSE SRC_FILES CMAKE_CONFIGURE_DEPENDS "engine/*.cpp" "engine/*.h")

if (APPLE)
  file(GLOB_RECURSE APPLE_FILES CMAKE_CONFIGURE_DEPENDS "engine/*.mm")
endif()

# Define the executable for the program
add_library(Vision ${SRC_FILES} ${APPLE_FILES})

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
                          glslang-default-resource-limits
                          metal-cpp)
