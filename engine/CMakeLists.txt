
set(LISE_HEADERS
	container/darray.h
	container/vector2.h
	core/application.h
	core/clock.h
	core/event.h
	core/input.h
	core/logger.h
	platform/platform.h
	renderer/command_buffer.h
	renderer/device.h
	renderer/fence.h
	renderer/framebuffer.h
	renderer/render_pass.h
	renderer/renderer.h
	renderer/swapchain.h
	renderer/vulkan_backend.h
	renderer/vulkan_image.h
	renderer/vulkan_platform.h
	util/math_utils.h
	util/string_utils.h
)
list(TRANSFORM LISE_HEADERS PREPEND "include/")

set(LISE_SOURCES
	container/darray.c
	container/vector2.c
	core/application.c
	core/clock.c
	core/event.c
	core/input.c
	core/logger.c
	platform/platform_linux.c
	platform/platform_win32.c
	renderer/command_buffer.c
	renderer/device.c
	renderer/fence.c
	renderer/framebuffer.c
	renderer/render_pass.c
	renderer/renderer.c
	renderer/swapchain.c
	renderer/vulkan_backend.c
	renderer/vulkan_image.c
	util/math_utils.c
	util/string_utils.c
)
list(TRANSFORM LISE_SOURCES PREPEND "src/")

add_library(lise SHARED ${LISE_SOURCES} ${LISE_HEADERS})
target_include_directories(lise PUBLIC include/)
target_compile_definitions(lise PRIVATE L_EXPORT)
target_compile_options(lise PRIVATE -fvisibility=hidden)

#if (CMAKE_BUILD_TYPE MATCHES "Release")
#	target_link_libraries (lise -static-libgcc -static)
#endif (CMAKE_BUILD_TYPE MATCHES "Release")

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

	#	if(NOT EXISTS "${CMAKE_BINARY_DIR}/resources")
	#		message(STATUS "Creating symlink for resources.")
	#
	#		file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/resources" SYM_SHADER_LINK)
	#		file(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}/resources" SYM_SHADER_TARGET)
	#	
	#		execute_process(
	#			COMMAND cmd.exe /c mklink /J "${SYM_SHADER_LINK}" "${SYM_SHADER_TARGET}"
	#		)
	#	endif()

endif(WIN32)
if (UNIX)

	target_compile_definitions(lise PUBLIC L_ISLINUX)

	message(STATUS "Generating build files specifically for linux.")

	target_link_libraries(lise vulkan xcb X11-xcb X11)
	

	#	if(NOT EXISTS "${CMAKE_BINARY_DIR}/resources")
	#		message(STATUS "Creating symlink for resources.")
	#
	#		target_link_libraries(lise vulkan)
	#
	#		execute_process(
	#			COMMAND ln -s ${CMAKE_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/resources
	#		)
	#	endif()

endif (UNIX)
