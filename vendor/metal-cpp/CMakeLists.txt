cmake_minimum_required(VERSION 3.20)
project(metal-cpp)

set(BUILD_SHARED_LIBS off)

message(VERBOSE "Building Shared metal-cpp Libs: " ${BUILD_SHARED_LIBS})

add_library(metal-cpp metal-cpp.cpp)
target_include_directories(metal-cpp PUBLIC lib/)