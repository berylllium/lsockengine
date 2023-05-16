#pragma once

#include <vulkan/vulkan.h>

#include "definitions.hpp"
#include "math/vector2.hpp"
#include "renderer/device.hpp"
#include "renderer/command_buffer.hpp"

namespace lise
{

class VulkanImage
{
public:
	const vector2ui size;

	VulkanImage(
		const Device& device,
		VkImageType image_type,
		vector2ui size,
		VkFormat image_format,
		VkImageTiling image_tiling,
		VkImageUsageFlags use_flags,
		VkMemoryPropertyFlags memory_flags,
		bool create_view,
		VkImageAspectFlags view_aspect_flags
	);

	VulkanImage(VulkanImage&& other);

	VulkanImage(VulkanImage&) = delete; // Prevent copies.

	~VulkanImage();

	VulkanImage& operator = (const VulkanImage&) = delete; // Prevent copies.

	void transition_layout(const CommandBuffer& command_buffer, VkImageLayout old_layout, VkImageLayout new_layout);

	void copy_from_buffer(const CommandBuffer& command_buffer, VkBuffer buffer);

	VkImageView get_image_view() const;

private:
	VkImage image_handle = nullptr;

	VkDeviceMemory memory = nullptr;
	VkImageView image_view = nullptr;

	VkFormat image_format;

	const Device& device;
};

}
