cmake_minimum_required(VERSION 3.20)
project(DaedalusEngine VERSION 0.1.0 LANGUAGES CXX)

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output to Build/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Packages
find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

# Public Headers
include_directories(Include)

# Sub-folders
add_subdirectory(Engine)
add_subdirectory(Editor)

# Log
message(STATUS "Engine project initialized. Root: ${CMAKE_SOURCE_DIR}")