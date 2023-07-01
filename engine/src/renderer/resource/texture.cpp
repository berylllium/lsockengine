#include "renderer/resource/texture.hpp"

//#include <stdlib.h>
//#include <string.h>

#include "core/logger.hpp"
#include "renderer/vulkan_buffer.hpp"

namespace lise
{

Texture::Texture(
	const Device& device,
	const std::string& path,
	vector2ui size,
	uint32_t channel_count,
	const uint8_t* data,
	bool has_transparency
) : device(device), path(path), size(size), channel_count(channel_count)
{
	uint64_t byte_size = size.w * size.h * channel_count;

	// Assume format.
	VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;
	
	// Create a staging buffer and load texture data into it.
	VulkanBuffer staging_buffer(
		device,
		device.get_memory_properties(),
		byte_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		true
	);

	staging_buffer.load_data(0, byte_size, 0, data);

	// Create the image. A lot of assumptions are made here to work with the PNG format.
	image = new VulkanImage(
		device,
		VK_IMAGE_TYPE_2D,
		size,
		image_format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		true,
		VK_IMAGE_ASPECT_COLOR_BIT
	);

	// Create and begin a temporary buffer.
	CommandBuffer temp_buffer(device, device.get_graphics_command_pool(), true);
	temp_buffer.begin(true, false, false);

	// Transition the image layout from undefined to optimal for receiving data.
	image->transition_layout(temp_buffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// Copy the data from the staging buffer to the image buffer.
	image->copy_from_buffer(temp_buffer, staging_buffer);

	// Transition the image layout from optimal for receiving data to a shader read only optimal layout.
	image->transition_layout(
		temp_buffer,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);

	// End and submit the temporary command buffer.
	temp_buffer.end_and_submit_single_use(device.get_graphics_queue());

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

	if (vkCreateSampler(device, &sampler_ci, NULL, &sampler) != VK_SUCCESS)
	{
		LERROR("Failed to create a sampler for the following texture: `%s`.", path);

		delete image;
		throw std::exception();
	}
}

Texture::Texture(Texture&& other) : device(other.device), path(other.path), size(other.size),
	channel_count(other.channel_count)
{
	image = other.image;
	other.image = nullptr;

	sampler = other.sampler;
	other.sampler = nullptr;
}

Texture::~Texture()
{
	delete image;

	if (sampler)
	{
		vkDestroySampler(device, sampler, NULL);
	}
}

Texture::operator const VulkanImage&() const
{
	return *image;
}

VkImageView Texture::get_image_view() const
{
	return image->get_image_view();
}

VkSampler Texture::get_sampler() const
{
	return sampler;
}

}
