project (Waves)

# Source Files
file(GLOB_RECURSE SRC_FILES CMAKE_CONFIGURE_DEPENDS "waves/*.cpp" "waves/*.h")

add_executable(Waves ${SRC_FILES})

target_link_libraries(Waves 
                        PRIVATE
                          Vision
                          SDL3::SDL3
                          glad
                          glm::glm
                          ImGui
                          spirv-cross-core
                          spirv-cross-glsl)