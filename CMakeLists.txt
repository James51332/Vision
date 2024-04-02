# Basic Info
cmake_minimum_required(VERSION 3.20)

# Set the C++ Standard
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED True)

# Vendored SDL Library (Output to the same directory as project)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include (engine/CMakeLists.txt)
include (lumina/CMakeLists.txt)