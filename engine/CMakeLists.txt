
set(LISE_HEADERS
	core/clock.hpp
	core/engine.hpp
	core/event.hpp
	core/input.hpp
	core/logger.hpp
	math/vector2.hpp
	platform/platform.hpp
)
list(TRANSFORM LISE_HEADERS PREPEND "include/")

set(LISE_SOURCES
	core/clock.cpp
	core/engine.cpp
	core/event.cpp
	core/input.cpp
	core/logger.cpp
	platform/platform_linux.cpp
	platform/platform_win32.cpp
)
list(TRANSFORM LISE_SOURCES PREPEND "src/")

add_library(lise SHARED ${LISE_SOURCES} ${LISE_HEADERS})
target_include_directories(lise PUBLIC include/ ../deps/stb)
target_compile_definitions(lise PRIVATE L_EXPORT)
target_compile_options(lise PRIVATE -fvisibility=hidden)

target_link_libraries(lise m) # Link math

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
		file(TO_NATIVE_PATH "${CMAKE_CURRENT_LIST_DIR}/../assets" SYM_SHADER_TARGET)
	
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
			COMMAND ln -s ${CMAKE_CURRENT_LIST_DIR}/../assets ${CMAKE_BINARY_DIR}/assets
		)
	endif()

endif (UNIX)
