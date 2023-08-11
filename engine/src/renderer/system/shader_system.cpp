#include "renderer/system/shader_system.hpp"

#include <unordered_map>

#include <simple-logger.hpp>

namespace lise
{

static std::unordered_map<std::string, std::unique_ptr<Shader>> loaded_shaders;

// Caches
static const Device* p_device;
static const Swapchain* p_swapchain;

bool shader_system_initialize(const Device* device, const Swapchain* swapchain)
{
	// Set the caches.
	p_device = device;
	p_swapchain = swapchain;

	sl::log_info("Successfully initialized the renderer shader subsystem.");

	return true;
}

void shader_system_shutdown()
{
	// Destroy all shaders.
	loaded_shaders.clear();

	// Clear caches.
	p_device = NULL;
	p_swapchain = NULL;

	sl::log_info("Successfully shut down the renderer shader subsystem.");
}

void shader_system_update_cache(const Swapchain* swapchain)
{
	p_swapchain = swapchain;
}

Shader* shader_system_load(const std::string& path, const RenderPass* render_pass)
{
	if (loaded_shaders.contains(path))
	{
		// Shader is already loaded.
		sl::log_warn("Attempting to load an already loaded shader.");
		return nullptr;
	}

	// Load config.
	ShaderConfig shader_config;

	if (!shader_config_load(path, shader_config))
	{
		sl::log_error("Failed to load shader configuration file for shader `{}`.", path);

		return nullptr;
	}

	auto shader = Shader::create(
		p_device,
		shader_config,
		render_pass,
		p_swapchain->swapchain_info.swapchain_extent.width,
		p_swapchain->swapchain_info.swapchain_extent.height,
		p_swapchain->images.size()
	);
	
	if(!shader)
	{
		sl::log_error("Failed to load shader.");

		return nullptr;
	}

	auto& i_result = *loaded_shaders.insert({ path, std::move(shader) }).first;

	return i_result.second.get();
}

Shader* shader_system_get(const std::string& path)
{
	std::unordered_map<std::string, std::unique_ptr<Shader>>::iterator it = loaded_shaders.find(path);

	if (it == loaded_shaders.end())
	{
		sl::log_error("Failed to find shader with path `{}`.", path);
		return nullptr;
	}

	auto& p = *it;

	return p.second.get();
}

}
