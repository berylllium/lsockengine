
set(LISE_SOURCES
	platform/platform_win32.c
	core/logger.c
)
list(TRANSFORM LISE_SOURCES PREPEND "src/")

add_library(lise SHARED ${LISE_SOURCES})
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

