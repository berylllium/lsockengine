#pragma once

#include "definitions.h"
#include "renderer/shader.h"
#include "renderer/device.h"
#include "renderer/render_pass.h"

/**
 * @brief Initializes the shader system. The shader system loads and caches shaders, and allows the user to retrieve
 * pointers to cached shaders.
 * 
 * @param device A pointer to the lise_device currently in use by the engine. This pointer gets cached internally, so
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
bool lise_shader_system_initialize(
	const lise_device* device,
	const lise_render_pass* world_render_pass,
	const uint32_t* framebuffer_width,
	const uint32_t* framebuffer_height,
	const uint32_t* swapchain_image_count
);

void lise_shader_system_shutdown();

bool lise_shader_system_load(const char* path, lise_shader** out_shader);

bool lise_shader_system_get(const char* path, lise_shader** out_shader);

bool lise_shader_system_get_or_load(const char* path, lise_shader** out_shader);
