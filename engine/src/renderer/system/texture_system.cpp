#include "renderer/system/texture_system.hpp"

//#include <stdlib.h>
#include <map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "core/logger.hpp"

namespace lise
{

static std::map<std::string, Texture> loaded_textures;

static const char* default_texture_path = "__default_texture_path__";
static Texture* default_texture;

static bool create_default_texture(const Device& device);

bool texture_system_initialize(const Device& device)
{
	stbi_set_flip_vertically_on_load(true);

	// Create the default texture.
	if (!create_default_texture(device))
	{
		LFATAL("Failed to create the default texture.");
		return false;
	}
	
	LINFO("Successfully initialized the renderer texture subsystem.");

	return true;
}

void texture_system_shutdown(const Device& device)
{
	// Free all textures.
	loaded_textures.clear();

	// Destroy the default texture.
	delete default_texture;

	LINFO("Successfully shut down the renderer texture subsystem.");
}

const Texture* texture_system_get_default_texture()
{
	return default_texture;
}

const Texture* texture_system_load(const Device& device, const std::string& path)
{
	if (loaded_textures.contains(path))
	{
		// Texture is already loaded.
		LWARN("Attempting to load an already loaded texture.");
		return default_texture;
	}

	// Load the image using stb_image.
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t channel_count = 0;
	constexpr uint32_t required_channel_count = 4;

	uint8_t* data = stbi_load(path.c_str(), (int*) &width, (int*) &height, (int*) &channel_count, STBI_rgb_alpha);

	if (!data)
	{
		LERROR(
			"STB_image failed to load an image during texture creation of texture `%s`. Error: %s.",
			path,
			stbi_failure_reason()
		);

		return default_texture;
	}

	uint64_t size = width * height * required_channel_count;

	// Check for transparency.
	bool has_transparency = false;

	for (uint64_t i = 0 ; i < size; i += required_channel_count)
	{
		if (data[i + 4] < 255)
		{
			has_transparency = true;
			break;
		}
	}

	// Create texture.
	try
	{
		// TODO: Use std::piecewise_construct
		loaded_textures.emplace(path, std::move(Texture(device, path, vector2ui { width, height} , channel_count, data, has_transparency)));
	}
	catch (std::exception e)
	{
		LERROR("Faild to load texture: `%s`. Providing default texture.", path);

		return default_texture;
	}

	return &loaded_textures.at(path);
}

const Texture* texture_system_get(const std::string& path)
{
	if (!loaded_textures.contains(path))
	{
		LWARN("Texture with path `%s` has not been loaded yet. Providing default texture.", path);

		return default_texture;
	}

	return &loaded_textures.at(path);
}

const Texture* texture_system_get_or_load(const Device& device, const std::string& path)
{
	if (loaded_textures.contains(path))
	{
		// Texture has already been loaded.
		return &loaded_textures.at(path);
	}
	else
	{
		return texture_system_load(device, path);
	}
}

// Static helper functions.
static bool create_default_texture(const Device& device)
{
	LINFO("Creating the default texture.");

	const uint32_t texture_dims = 256;
	const uint32_t half = texture_dims / 2;
	const uint32_t channels = 4;
	const uint32_t size = texture_dims * texture_dims * channels;

	std::unique_ptr<std::uint8_t[]> data = std::make_unique<std::uint8_t[]>(size);

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

	try
	{
		default_texture = new Texture(
			device,
			default_texture_path,
			vector2ui { texture_dims, texture_dims },
			channels,
			data.get(),
			false
		);
	}
	catch (std::exception)
	{
		LERROR("Failed to create texture object for the default texture.");
		return false;
	}

	return true;
}

}
