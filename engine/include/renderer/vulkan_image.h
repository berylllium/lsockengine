#pragma once

#include <vulkan/vulkan.h>

#include "definitions.h"
#include "renderer/device.h"

typedef struct lise_vulkan_image
{
	VkImage image_handle;
	VkDeviceMemory memory;
	VkImageView image_view;

	uint32_t width;
	uint32_t height;
} lise_vulkan_image;

bool lise_vulkan_image_create(
	lise_device* device,
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
	