#include "renderer/system/shader_system.h"

#include <stdlib.h>

#include <container/hashmap.h>

#include "core/logger.h"

static blib_hashmap loaded_shaders;

// Caches
static const lise_device* p_device;
static const lise_render_pass* p_world_render_pass;
static const uint32_t* p_framebuffer_width;
static const uint32_t* p_framebuffer_height;
static const uint32_t* p_swapchain_image_count;

bool lise_shader_system_initialize(
	const lise_device* device,
	const lise_render_pass* world_render_pass,
	const uint32_t* framebuffer_width,
	const uint32_t* framebuffer_height,
	const uint32_t* swapchain_image_count
)
{
	// Set the caches.
	p_device = device;
	p_world_render_pass = world_render_pass;
	p_framebuffer_width = framebuffer_width;
	p_framebuffer_height = framebuffer_height;
	p_swapchain_image_count = swapchain_image_count;

	// Initialize the loaded shaders hashmap.
	loaded_shaders = blib_hashmap_create();

	LINFO("Successfully initialized the renderer shader subsystem.");

	return true;
}

void lise_shader_system_shutdown()
{
	// Loop through the hashmap and free all shaders.
	for (uint64_t i = 0; i < loaded_shaders.base_bucket_capacity; i++)
	{
		blib_hashmap_bucket* bucket = loaded_shaders.buckets[i];

		while (bucket)
		{
			lise_shader_destroy(p_device->logical_device, bucket->value);

			bucket = bucket->next;
		}
	}

	blib_hashmap_free(&loaded_shaders);

	// Clear caches.
	p_device = NULL;
	p_world_render_pass = NULL;
	p_framebuffer_width = NULL;
	p_framebuffer_height = NULL;
	p_swapchain_image_count = NULL;

	LINFO("Successfully shut down the renderer shader subsystem.");
}

bool lise_shader_system_load(const char* path, lise_shader** out_shader)
{
	if (blib_hashmap_get(&loaded_shaders, path))
	{
		// Shader is already loaded.
		LWARN("Attempting to load an already loaded shader.");
		return false;
	}

	lise_shader* new_shader = malloc(sizeof(lise_shader));

	if (!lise_shader_create(
		p_device->logical_device,
		p_device->physical_device_memory_properties,
		p_device->physical_device_properties.limits.minUniformBufferOffsetAlignment,
		path,
		p_world_render_pass,
		*p_framebuffer_width,
		*p_framebuffer_height,
		*p_swapchain_image_count,
		new_shader
	))
	{
		LERROR("Failed to load shader.");
		return false;
	}

	blib_hashmap_set(&loaded_shaders, path, new_shader);

	if (out_shader) *out_shader = new_shader;

	return true;
}

bool lise_shader_system_get(const char* path, lise_shader** out_shader)
{
	lise_shader* found_shader = blib_hashmap_get(&loaded_shaders, path);

	if (!found_shader)
	{
		LERROR("Failed to find shader with path `%s`.", path);
		return false;
	}

	if (out_shader) *out_shader = found_shader;

	return true;
}

bool lise_shader_system_get_or_load(const char* path, lise_shader** out_shader)
{
	lise_shader* found_shader = blib_hashmap_get(&loaded_shaders, path);

	if (found_shader)
	{
		if (out_shader) *out_shader = found_shader;

		return true;
	}
	else
	{
		return lise_shader_system_load(path, out_shader);
	}
}
