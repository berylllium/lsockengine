#pragma once

#include <vulkan/vulkan.h>

#include "definitions.h"
#include "renderer/device.h"
#include "renderer/command_buffer.h"

typedef struct lise_vulkan_image
{
	VkImage image_handle;
	VkDeviceMemory memory;
	VkImageView image_view;

	uint32_t width;
	uint32_t height;
} lise_vulkan_image;

bool lise_vulkan_image_create(
	const lise_device* device,
	VkImageType image_type,
	uint32_t width,
	uint32_t height,
	VkFormat image_format,
	VkImageTiling image_tiling,
	VkImageUsageFlags use_flags,
	VkMemoryPropertyFlags memory_flags,
	bool create_view,
	VkImageAspectFlags view_aspect_flags,
	lise_vulkan_image* out_image
);

void lise_vulkan_image_destroy(VkDevice device, lise_vulkan_image* image);

void lise_vulkan_image_transition_layout(
	lise_command_buffer* command_buffer,
	lise_vulkan_image* image,
	VkFormat format,
	VkImageLayout old_layout,
	VkImageLayout new_layout
);

void lise_vulkan_image_copy_from_buffer(lise_command_buffer* command_buffer, VkBuffer buffer, lise_vulkan_image* image);
	