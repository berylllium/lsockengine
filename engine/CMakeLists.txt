
set(LISE_SOURCES
	core/clock.cpp
	core/engine.cpp
	core/event.cpp
	core/input.cpp
	loader/obj_format_loader.cpp
	loader/obj_loader.cpp
	loader/shader_config_loader.cpp
	math/mat4x4.cpp
	math/math.cpp
	math/transform.cpp
	node/mesh_renderer3d.cpp
	node/node_tree.cpp
	node/node.cpp
	node/node3d.cpp
	node/view_port.cpp
	node/visual_node3d.cpp
	platform/platform_linux.cpp
	platform/platform_win32.cpp
	renderer/resource/mesh.cpp
	renderer/resource/model.cpp
	renderer/resource/shader_stage.cpp
	renderer/resource/shader.cpp
	renderer/resource/texture.cpp
	renderer/system/shader_system.cpp
	renderer/system/texture_system.cpp
	renderer/command_buffer.cpp
	renderer/device.cpp
	renderer/fence.cpp
	renderer/pipeline.cpp
	renderer/render_pass.cpp
	renderer/renderer.cpp
	renderer/swapchain.cpp
	renderer/vulkan_backend.cpp
	renderer/vulkan_buffer.cpp
	renderer/vulkan_image.cpp
	util/string_utils.cpp
)
list(TRANSFORM LISE_SOURCES PREPEND "src/")

add_library(lise SHARED ${LISE_SOURCES})
target_include_directories(lise PUBLIC include/ ../deps/stb)
target_compile_definitions(lise PRIVATE L_EXPORT PUBLIC VULKAN_HPP_NO_EXCEPTIONS)
target_compile_options(lise PRIVATE -fvisibility=hidden PUBLIC -fno-exceptions)

target_link_libraries(lise PUBLIC m PUBLIC simple-logger) # Link math

if (CMAKE_BUILD_TYPE MATCHES "Release")
	target_link_libraries (lise PUBLIC -static-libgcc PUBLIC -static)
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
	target_link_libraries(lise PUBLIC ${VULKAN_PATH}/Lib/vulkan-1.lib)

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

	target_link_libraries(lise PUBLIC vulkan PUBLIC xcb PUBLIC X11-xcb PUBLIC X11)
	
	# Create symlink to assets folder in build path
	if(NOT EXISTS "${CMAKE_BINARY_DIR}/assets")
		message(STATUS "Creating symlink for assets.")
	
		execute_process(
			COMMAND ln -s ${CMAKE_CURRENT_LIST_DIR}/../assets ${CMAKE_BINARY_DIR}/assets
		)
	endif()

endif (UNIX)
