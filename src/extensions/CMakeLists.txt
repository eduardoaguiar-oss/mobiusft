# Specify the minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Define the project for extensions
project(extensions LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define top-level source and build directories
set(TOP_SRC_DIR ${CMAKE_SOURCE_DIR}/..)
set(TOP_BUILD_DIR ${CMAKE_BINARY_DIR}/..)

# Build extensions
add_subdirectory(app/emule)
add_subdirectory(event-viewer)
