cmake_minimum_required(VERSION 3.20)
project(stb)

set(BUILD_SHARED_LIBS off)

message(VERBOSE "Building Shared STB Libs: " ${BUILD_SHARED_LIBS})

add_library(stb stb_image.cpp stb_image.h)
target_include_directories(stb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})