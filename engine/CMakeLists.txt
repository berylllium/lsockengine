
set(LISE_HEADERS
	core/application.h
	core/clock.h
	core/event.h
	core/input.h
	core/logger.h
	loader/obj_format_loader.h
	loader/obj_loader.h
	loader/shader_config_loader.h
	math/mat4x4.h
	math/math.h
	math/quat.h
	math/transform.h
	math/vector2.h
	math/vector3.h
	math/vector4.h
	math/vertex.h
	platform/filesystem.h
	platform/platform.h
	renderer/resource/mesh.h
	renderer/resource/model.h
	renderer/resource/texture.h
	renderer/system/shader_system.h
	renderer/system/texture_system.h
	renderer/command_buffer.h
	renderer/device.h
	renderer/fence.h
	renderer/framebuffer.h
	renderer/pipeline.h
	renderer/render_pass.h
	renderer/renderer.h
	renderer/shader_stage.h
	renderer/shader.h
	renderer/swapchain.h
	renderer/vulkan_backend.h
	renderer/vulkan_buffer.h
	renderer/vulkan_image.h
	renderer/vulkan_platform.h
	util/math_utils.h
	util/string_utils.h
)
list(TRANSFORM LISE_HEADERS PREPEND "include/")

set(LISE_SOURCES
	core/application.c
	core/clock.c
	core/event.c
	core/input.c
	core/logger.c
	loader/obj_format_loader.c
	loader/obj_loader.c
	loader/shader_config_loader.c
	math/math.c
	math/transform.c
	platform/filesystem.c
	platform/platform_linux.c
	platform/platform_win32.c
	renderer/resource/mesh.c
	renderer/resource/model.c
	renderer/resource/texture.c
	renderer/system/shader_system.c
	renderer/system/texture_system.c
	renderer/command_buffer.c
	renderer/device.c
	renderer/fence.c
	renderer/framebuffer.c
	renderer/pipeline.c
	renderer/render_pass.c
	renderer/renderer.c
	renderer/shader_stage.c
	renderer/shader.c
	renderer/swapchain.c
	renderer/vulkan_backend.c
	renderer/vulkan_buffer.c
	renderer/vulkan_image.c
	util/math_utils.c
	util/string_utils.c
)
list(TRANSFORM LISE_SOURCES PREPEND "src/")

add_library(lise SHARED ${LISE_SOURCES} ${LISE_HEADERS})
target_include_directories(lise PUBLIC include/ ../deps/blib/include ../deps/stb)
target_compile_definitions(lise PRIVATE L_EXPORT)
target_compile_options(lise PRIVATE -fvisibility=hidden)

target_link_libraries(lise blib m) # Link blib, math

if (CMAKE_BUILD_TYPE MATCHES "Release")
	target_link_libraries (lise -static-libgcc -static)
endif (CMAKE_BUILD_TYPE MATCHES "Release")

if (WIN32)

	target_compile_definitions(lise PUBLIC L_ISWIN)

	message(STATUS "Generating build files specifically for windows.")

	message(STATUS "Attempting to locate Vulkan SDK using manual path...")
	set(VULKAN_SDK "C:/VulkanSDK")
	set(VULKAN_VERSION "1.3.231.1")
	set(VULKAN_PATH "${VULKAN_SDK}/${VULKAN_VERSION}")
	message(STATUS "Using manually specified path: ${VULKAN_PATH}")

	# Check if manually set path exists
	if(NOT EXISTS "${VULKAN_PATH}")
		message("Error: Unable to locate this Vulkan SDK path VULKAN_PATH: ${VULKAN_PATH}, please specify correct path.")
		return()
	endif()

	target_include_directories(lise PUBLIC ${VULKAN_PATH}/Include)
	target_link_libraries(lise ${VULKAN_PATH}/Lib/vulkan-1.lib)

	# Create symlink to assets folder in build path
	if(NOT EXISTS "${CMAKE_BINARY_DIR}/assets")
		message(STATUS "Creating symlink for assets.")
	
		file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/assets" SYM_SHADER_LINK)
		file(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}/assets" SYM_SHADER_TARGET)
	
		execute_process(
			COMMAND cmd.exe /c mklink /J "${SYM_SHADER_LINK}" "${SYM_SHADER_TARGET}"
		)
	endif()

endif(WIN32)
if (UNIX)

	target_compile_definitions(lise PUBLIC L_ISLINUX)

	message(STATUS "Generating build files specifically for linux.")

	target_link_libraries(lise vulkan xcb X11-xcb X11)
	
	# Create symlink to assets folder in build path
	if(NOT EXISTS "${CMAKE_BINARY_DIR}/assets")
		message(STATUS "Creating symlink for assets.")
	
		target_link_libraries(lise vulkan)
	
		execute_process(
			COMMAND ln -s ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets
		)
	endif()

endif (UNIX)
