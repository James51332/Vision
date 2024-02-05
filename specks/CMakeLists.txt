# Basic Info
project(Specks)

# Source Files
file(GLOB_RECURSE SRC_FILES CMAKE_CONFIGURE_DEPENDS "specks/*.cpp" "specks/*.h specks/**.cpp specks/**.h")

# Define the executable for the program
add_executable(Specks ${SRC_FILES})

include_directories("specks")

# Link to the SDL library
target_link_libraries(Specks 
                        PRIVATE
                          Vision
                          SDL3::SDL3
                          glad
                          glm::glm
                          ImGui
                          spirv-cross-core
                          spirv-cross-glsl)