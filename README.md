# LiSE
A PS1-style game engine.

## Support

LiSE provides native support for Windows and GNU/Linux systems using the Xorg window system. Support for MacOs is not planned.

### Windows

- Download and install the [Vulkan SDK](https://vulkan.lunarg.com/). (Preferably the same version used in the project, currently version 1.3.231.1. You could probably use more recent versions.)
- Change the `VULKAN_SDK` variable in the CMakeLists file to where you've installed the SDK. Also change the `VULKAN_VERSION` variable if you've chosen to go for a more recent version.
- Configure the CMake project and build it.

### GNU/Linux

The following instructions apply to Arch Linux. Other distributions have different names for the packages.

- Install the following packages from pacman: `vulkan-devel shaderc`. (`vulkan-devel` contains everything needed for vulkan development, including Vulkan headers and the library required for linking. The `shaderc` package contains the `glslc` binary used to compile GLSL code into SPIR-V bytecode.)
- Configure the CMake project and build it.

> **__NOTE:__** Please read through the [Notice](#notice) section for more information about the quirks running LiSE on GNU/Linux using the Xorg window system.

## Notice

LiSE uses `= {}` struct initialization. At the time of writing, this is a GCC extension. C23 will include this as standard in the future.

Xorg does not support disabling key-repeats for specific windows. This means that when one wants to disable key-repeats they have to do so for the entire window system. Meaning LiSE will disable key-repeats for the entire system and reenable it when closing down. This also means that key-repeats will remain disabled if the program closes without successfully going through the shutdown procedure; this may occur on crashes.
