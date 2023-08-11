#pragma once

#include <string>

#include "definitions.hpp"
#include "renderer/resource/shader.hpp"
#include "renderer/device.hpp"
#include "renderer/render_pass.hpp"
#include "renderer/swapchain.hpp"

namespace lise
{

/**
 * @brief Initializes the shader system. The shader system loads and caches shaders, and allows the user to retrieve
 * pointers to cached shaders.
 * 
 * @param device A pointer to the device currently in use by the engine. This pointer gets cached internally, so
 * make sure to keep using the same memory address for the device.
 * @param swapchain A pointer to the swapchain. This pointer gets cached internally.
 * 
 * @return true if the initialisation succeeded.
 * @return false if the initialisation failed.
 */
bool shader_system_initialize(const Device* device, const Swapchain* swapchain);

void shader_system_shutdown();

void shader_system_update_cache(const Swapchain* swapchain);

Shader* shader_system_load(const std::string& path, const RenderPass* render_pass);

Shader* shader_system_get(const std::string& path);

}
