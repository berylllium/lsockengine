#pragma once

#include <string>

#include "definitions.hpp"
#include "renderer/resource/shader.hpp"
#include "renderer/device.hpp"
#include "renderer/render_pass.hpp"

namespace lise
{

/**
 * @brief Initializes the shader system. The shader system loads and caches shaders, and allows the user to retrieve
 * pointers to cached shaders.
 * 
 * @param device A pointer to the device currently in use by the engine. This pointer gets cached internally, so
 * make sure to keep using the same memory address for the device.
 * @param world_render_pass A pointer to the world render pass. This pointer gets cached internally, so make sure to
 * keep using the same memory address for this render pass.
 * @param framebuffer_width A pointer to the framebuffer width. This pointer gets cached internally.
 * @param framebuffer_height A pointer to the framebuffer height. This pointer gets cached internally.
 * @param swapchain_image_count A pointer to the swapchain image count. This pointer gets cached internally.
 * 
 * @return true if the initialisation succeeded.
 * @return false if the initialisation failed.
 */
bool shader_system_initialize(
	const Device& device,
	const RenderPass& world_render_pass,
	const uint32_t& framebuffer_width,
	const uint32_t& framebuffer_height,
	const uint32_t& swapchain_image_count
);

void shader_system_shutdown();

const Shader* shader_system_load(const std::string& path);

const Shader* shader_system_get(const std::string& path);

const Shader* shader_system_get_or_load(const std::string& path);

}
