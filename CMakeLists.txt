cmake_minimum_required(VERSION 3.21)

project(lise)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

add_subdirectory(engine)
add_subdirectory(test)

add_subdirectory(deps/blib)
