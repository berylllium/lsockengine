# LiSE
A PS1-style game engine.

## Support

LiSE provides native support for Windows and GNU/Linux systems. Support for MacOs is not planned.

### Windows

- Download and install the [Vulkan SDK](https://vulkan.lunarg.com/). (Preferably the same version used in the project, currently version 1.3.231.1. You could probably use more recent versions.)
- Change the `VULKAN_SDK` variable in the CMakeLists file to where you've installed the SDK. Also change the `VULKAN_VERSION` variable if you've chosen to go for a more recent version.
- Configure the CMake project and build it.

### GNU/Linux

The following instructions apply to Arch Linux. Other distributions have different names for the packages.

- Install the following packages from pacman: `vulkan-devel shaderc`. (`vulkan-devel` contains everything needed for vulkan development, including Vulkan headers and the library required for linking. The `shaderc` package contains the `glslc` binary used to compile GLSL code into SPIR-V bytecode.)
- Configure the CMake project and build it.

## Notice

LiSE uses `= {}` struct initialization. At the time of writing, this is a GCC extension. C23 will
include this as standard in the future.
