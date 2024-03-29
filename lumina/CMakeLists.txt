project (Lumina)

# Source Files
file(GLOB_RECURSE SRC_FILES CMAKE_CONFIGURE_DEPENDS "lumina/*.cpp" "lumina/*.h")

add_executable(Lumina ${SRC_FILES})

target_link_libraries(Lumina 
                        PRIVATE
                          Vision)