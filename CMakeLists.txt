cmake_minimum_required(VERSION 3.21)

project(lise)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

set (CMAKE_CXX_STANDARD 20)

add_subdirectory(engine)
add_subdirectory(test)
