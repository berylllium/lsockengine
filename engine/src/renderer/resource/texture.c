#include "renderer/resource/texture.h"

#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "core/logger.h"
#include "renderer/vulkan_buffer.h"

bool lise_texture_create_from_path(const lise_device* device, const char* path, lise_texture* out_texture)
{
	stbi_set_flip_vertically_on_load(true);

	// Load the image using stb_image.
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t channel_count = 0;
	const uint32_t required_channel_count = 4;

	uint8_t* data = stbi_load(path, (int*) &width, (int*) &height, (int*) &channel_count, STBI_rgb_alpha);

	if (!data)
	{
		LERROR(
			"STB_image failed to load an image during texture creation of texture `%s`. Error: %s.",
			path,
			stbi_failure_reason()
		);

		return false;
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

	bool ret = lise_texture_create(
		device,
		path,
		width,
		height,
		required_channel_count,
		data,
		has_transparency,
		out_texture
	);

	// Free up the stbi data as we no longer need it.
	stbi_image_free(data);

	return ret;
}

bool lise_texture_create(
	const lise_device* device,
	const char* path,
	uint32_t width,
	uint32_t height,
	uint32_t channel_count,
	const uint8_t* data,
	bool has_transparency,
	lise_texture* out_texture
)
{
	uint64_t size = width * height * channel_count;

	// Assume format.
	VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;
	
	// Create a staging buffer and load texture data into it.
	lise_vulkan_buffer staging_buffer;
	lise_vulkan_buffer_create(
		device->logical_device,
		device->physical_device_memory_properties,
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		true,
		&staging_buffer
	);

	lise_vulkan_buffer_load_data(device->logical_device, &staging_buffer, 0, size, 0, data);

	// Create the image. A lot of assumptions are made here to work with the PNG format.
	lise_vulkan_image_create(
		device,
		VK_IMAGE_TYPE_2D,
		width,
		height,
		image_format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		true,
		VK_IMAGE_ASPECT_COLOR_BIT,
		&out_texture->image
	);

	// Create and begin a temporary buffer.
	lise_command_buffer temp_buffer;
	lise_command_buffer_allocate_and_begin_single_use(
		device->logical_device,
		device->graphics_command_pool,
		&temp_buffer
	);

	// Transition the image layout from undefined to optimal for receiving data.
	lise_vulkan_image_transition_layout(
		&temp_buffer,
		&out_texture->image,
		image_format,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);

	// Copy the data from the staging buffer to the image buffer.
	lise_vulkan_image_copy_from_buffer(
		&temp_buffer,
		staging_buffer.handle,
		&out_texture->image
	);

	// Transition the image layout from optimal for receiving data to a shader read only optimal layout.
	lise_vulkan_image_transition_layout(
		&temp_buffer,
		&out_texture->image,
		image_format,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);

	// End and submit the temporary command buffer.
	lise_command_buffer_end_and_submit_single_use(
		device->logical_device,
		device->graphics_command_pool,
		&temp_buffer,
		device->graphics_queue
	);

	// Destroy the staging buffer as we do not need it anymore.
	lise_vulkan_buffer_destroy(device->logical_device, &staging_buffer);

	// Create a sampler for the texture.
	VkSamplerCreateInfo sampler_ci = {};
	sampler_ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_ci.magFilter = VK_FILTER_LINEAR;
	sampler_ci.minFilter = VK_FILTER_LINEAR;
	sampler_ci.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_ci.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_ci.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_ci.anisotropyEnable = VK_FALSE;
	sampler_ci.maxAnisotropy = 16;
	sampler_ci.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	sampler_ci.unnormalizedCoordinates = VK_FALSE;
	sampler_ci.compareEnable = VK_FALSE;
	sampler_ci.compareOp = VK_COMPARE_OP_ALWAYS;
	sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_ci.mipLodBias = 0.0f;
	sampler_ci.minLod = 0.0f;
	sampler_ci.maxLod = 0.0f;

	if (vkCreateSampler(device->logical_device, &sampler_ci, NULL, &out_texture->sampler) != VK_SUCCESS)
	{
		LERROR("Failed to create a sampler for the following texture: `%s`.", path);
		return false;
	}

	// Copy over texture data.
	// Create a copy of the path. +1 for the null-terminator.
	out_texture->path = malloc(strlen(path) + 1);
	memcpy(out_texture->path, path, strlen(path) + 1);

	out_texture->width = width;
	out_texture->height = height;
	out_texture->channel_count = channel_count;

	return true;
}

void lise_texture_free(VkDevice device, lise_texture* texture)
{
	vkDeviceWaitIdle(device);

	lise_vulkan_image_destroy(device, &texture->image);

	vkDestroySampler(device, texture->sampler, NULL);
	texture->sampler = NULL;

	// Free path.
	free(texture->path);
	texture->path = NULL;
}
