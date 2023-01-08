cmake_minimum_required (VERSION 3.12)

project (lise C)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

add_subdirectory(engine)
add_subdirectory(test)
