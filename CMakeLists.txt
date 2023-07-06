cmake_minimum_required(VERSION 3.21)

project(lise)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(engine)
add_subdirectory(test)
