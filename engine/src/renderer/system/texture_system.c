#include "renderer/system/texture_system.h"

#include <stdlib.h>

#include <container/hashmap.h>

#include "core/logger.h"

static blib_hashmap loaded_textures;

static const char* default_texture_path = "__lise_default_texture_path__";
static lise_texture default_texture;

static bool create_default_texture(const lise_device* device);

bool lise_texture_system_initialize(const lise_device* device)
{

	// Initialize the loaded textures hashmap.
	loaded_textures = blib_hashmap_create();

	// Create the default texture.
	if (!create_default_texture(device))
	{
		LFATAL("Failed to create the default texture.");
		return false;
	}
	
	LINFO("Successfully initialized the renderer texture subsystem.");

	return true;
}

void lise_texture_system_shutdown(VkDevice device)
{
	// Loop through the hashmap and free all textures.
	for (uint64_t i = 0; i < loaded_textures.base_bucket_capacity; i++)
	{
		blib_hashmap_bucket* bucket = loaded_textures.buckets[i];

		while (bucket)
		{
			lise_texture_free(device, bucket->value);

			bucket = bucket->next;
		}
	}

	blib_hashmap_free(&loaded_textures);

	// Destroy the default texture.
	lise_texture_free(device, &default_texture);

	LINFO("Successfully shut down the renderer texture subsystem.");
}

const lise_texture* lise_texture_system_get_default_texture()
{
	return &default_texture;
}

bool lise_texture_system_load(const lise_device* device, const char* path, lise_texture** out_texture)
{
	if (blib_hashmap_get(&loaded_textures, path))
	{
		// Texture is already loaded.
		LWARN("Attempting to load an already loaded texture.");
		return false;
	}

	lise_texture* new_texture = malloc(sizeof(lise_texture));

	if (!lise_texture_create_from_path(
		device,
		path,
		new_texture
	))
	{
		LERROR("Faild to load texture: `%s`.", path);

		free(new_texture);
		
		return false;
	}

	blib_hashmap_set(&loaded_textures, path, new_texture);

	*out_texture = new_texture;

	return true;
}

bool lise_texture_system_get(const lise_device* device, const char* path, lise_texture** out_texture)
{
	lise_texture* found_texture = blib_hashmap_get(&loaded_textures, path);

	if (!found_texture)
	{
		LWARN("Texture with path `%s` has not been loaded yet. Providing default texture.", path);

		*out_texture = &default_texture;

		return false;
	}

	*out_texture = found_texture;

	return true;
}

bool lise_texture_system_get_or_load(const lise_device* device, const char* path, lise_texture** out_texture)
{
	lise_texture* found_texture = blib_hashmap_get(&loaded_textures, path);

	if (found_texture)
	{
		// Texture has already been loaded.
		*out_texture = found_texture;

		return true;
	}
	else
	{
		return lise_texture_system_load(device, path, out_texture);
	}
}

// Static helper functions

static bool create_default_texture(const lise_device* device)
{
	LINFO("Creating the default texture.");

	const uint32_t texture_dims = 256;
	const uint32_t half = texture_dims / 2;
	const uint32_t channels = 4;
	const uint32_t size = texture_dims * texture_dims * channels;

	uint8_t* data = malloc(sizeof(uint8_t) * size);

	// Loop through every pixel and set the color. The default texture is a 256x256 texture divided in four quadrants,
	// and made up of two colors; magenta (255, 0, 255) and black (0, 0, 0). The quadrants are colored as follows: 
	// Top left: magenta. Top right: black. Bottom left: black. Bottom right: magenta.
	for (uint32_t y = 0; y < texture_dims; y++)
	{
		for (uint32_t x = 0; x < texture_dims; x++)
		{
			uint32_t pixel_idx = channels * (y * texture_dims + x);

			bool x_below = x < half;
			bool y_below = y < half;

			if ((x_below && y_below) || (!x_below && !y_below))
			{
				data[pixel_idx + 0] = 255;
				data[pixel_idx + 1] = 0;
				data[pixel_idx + 2] = 255;
				data[pixel_idx + 3] = 255;
			}
			else
			{
				data[pixel_idx + 0] = 0;
				data[pixel_idx + 1] = 0;
				data[pixel_idx + 2] = 0;
				data[pixel_idx + 3] = 255;
			}
		}
	}

	if (!lise_texture_create(
		device,
		default_texture_path,
		texture_dims,
		texture_dims,
		channels,
		data,
		false,
		&default_texture
	))
	{
		LERROR("Failed to create texture object for the default texture");

		free(data);

		return false;
	}

	free(data);

	return true;
}
