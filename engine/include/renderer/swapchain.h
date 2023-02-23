#pragma once

#include <vulkan/vulkan.h>

#include "renderer/device.h"
#include "renderer/vulkan_image.h"

#include "definitions.h"

typedef struct lise_swapchain
{
	VkSwapchainKHR swapchain_handle;

	VkSurfaceFormatKHR image_format;
	uint32_t image_count;
	VkImage* images;
	VkImageView* image_views;

	uint8_t max_frames_in_flight;
	uint8_t current_frame;
	
	lise_vulkan_image depth_attachment;
} lise_swapchain;

bool lise_swapchain_create(
	lise_device* device,
	VkSurfaceKHR surface,
	VkExtent2D window_extent,
	lise_swapchain* out_swapchain
);

void lise_swapchain_destroy(VkDevice device, lise_swapchain* swapchain);
