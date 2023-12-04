cmake_minimum_required(VERSION 3.10)
project(huffman)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

# Add executable
add_executable(huffman ${SOURCES})