project(Vision)

add_subdirectory(vendor/sdl2 EXCLUDE_FROM_ALL)
add_subdirectory(vendor/glad EXCLUDE_FROM_ALL)
add_subdirectory(vendor/glm EXCLUDE_FROM_ALL)
add_subdirectory(vendor/SPIRV-Cross EXCLUDE_FROM_ALL)
add_subdirectory(vendor/imgui EXCLUDE_FROM_ALL)

# Source Files
file(GLOB_RECURSE SRC_FILES CMAKE_CONFIGURE_DEPENDS "engine/*.cpp" "engine/*.h")

# Define the include directory for the program
include_directories(Vision ${CMAKE_SOURCE_DIR}/engine)

# Define the executable for the program
add_library(Vision ${SRC_FILES})

# Link to the SDL library
target_link_libraries(Vision
                        PRIVATE
                          SDL3::SDL3
                          glad
                          glm::glm
                          ImGui
                          spirv-cross-core
                          spirv-cross-glsl)
