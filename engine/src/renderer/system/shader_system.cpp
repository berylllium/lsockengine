#include "renderer/system/shader_system.hpp"

//#include <stdlib.h>

#include <unordered_map>

#include "core/logger.hpp"

namespace lise
{

static std::unordered_map<std::string, Shader> loaded_shaders;

// Caches
static const Device* p_device;
static const RenderPass* p_world_render_pass;
static const uint32_t* p_framebuffer_width;
static const uint32_t* p_framebuffer_height;
static const uint32_t* p_swapchain_image_count;

bool shader_system_initialize(
	const Device& device,
	const RenderPass& world_render_pass,
	const uint32_t& framebuffer_width,
	const uint32_t& framebuffer_height,
	const uint32_t& swapchain_image_count
)
{
	// Set the caches.
	p_device = &device;
	p_world_render_pass = &world_render_pass;
	p_framebuffer_width = &framebuffer_width;
	p_framebuffer_height = &framebuffer_height;
	p_swapchain_image_count = &swapchain_image_count;

	LINFO("Successfully initialized the renderer shader subsystem.");

	return true;
}

void shader_system_shutdown()
{
	// Destroy all shaders.
	loaded_shaders.clear();

	// Clear caches.
	p_device = NULL;
	p_world_render_pass = NULL;
	p_framebuffer_width = NULL;
	p_framebuffer_height = NULL;
	p_swapchain_image_count = NULL;

	LINFO("Successfully shut down the renderer shader subsystem.");
}

const Shader* shader_system_load(const std::string& path)
{
	if (loaded_shaders.contains(path))
	{
		// Shader is already loaded.
		LWARN("Attempting to load an already loaded shader.");
		return nullptr;
	}

	// Load config.
	ShaderConfig shader_config;

	if (!shader_config_load(path, shader_config))
	{
		LERROR("Failed to load shader configuration file for shader `%s`.", path);

		return nullptr;
	}

	std::unordered_map<std::string, Shader>::iterator it;
	bool ok;

	try
	{
		std::tie(it, ok) = loaded_shaders.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(path),
			std::forward_as_tuple(
				*p_device,
				shader_config,
				*p_world_render_pass,
				*p_framebuffer_width,
				*p_framebuffer_height,
				*p_swapchain_image_count
			)
		);
	}
	catch(std::exception e)
	{
		LERROR("Failed to load shader.");

		return nullptr;
	}

	return &((*it).second);
}

const Shader* shader_system_get(const std::string& path)
{
	std::unordered_map<std::string, Shader>::iterator it = loaded_shaders.find(path);

	if (it == loaded_shaders.end())
	{
		LERROR("Failed to find shader with path `%s`.", path);
		return nullptr;
	}

	return &((*it).second);
}

const Shader* shader_system_get_or_load(const std::string& path)
{
	std::unordered_map<std::string, Shader>::iterator it = loaded_shaders.find(path);

	if (it != loaded_shaders.end())
	{
		return &((*it).second);
	}
	else
	{
		return shader_system_load(path);
	}
}

}
