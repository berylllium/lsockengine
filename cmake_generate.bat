@ECHO OFF
REM CMake generation script because I'm too lazy to remember the commands.

PUSHD build
cmake -G "Ninja" -D CMAKE_C_COMPILER=gcc ..
POPD

