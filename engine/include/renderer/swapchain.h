#pragma once

#include <vulkan/vulkan.h>

#include "renderer/device.h"
#include "renderer/vulkan_image.h"
#include "renderer/framebuffer.h"

#include "definitions.h"

typedef struct lise_swapchain_info
{
	VkSurfaceFormatKHR image_format;
	VkPresentModeKHR present_mode;
	VkExtent2D swapchain_extent;

	VkFormat depth_format;

	uint32_t image_count;
} lise_swapchain_info;

typedef struct lise_swapchain
{
	VkSwapchainKHR swapchain_handle;

	VkSurfaceFormatKHR image_format;
	uint32_t image_count;
	VkImage* images;
	VkImageView* image_views;

	uint8_t max_frames_in_flight;
	uint8_t current_frame;
	
	VkFormat depth_format;
	lise_vulkan_image* depth_attachments;

	lise_framebuffer* framebuffers;

	bool swapchain_out_of_date;
} lise_swapchain;

bool lise_swapchain_create(
	lise_device* device,
	VkSurfaceKHR surface,
	lise_swapchain_info swapchain_info,
	lise_render_pass* render_pass,
	lise_swapchain* out_swapchain
);

bool lise_swapchain_recreate(
	lise_device* device,
	VkSurfaceKHR surface,
	lise_swapchain_info swapchain_info,
	lise_render_pass* render_pass,
	lise_swapchain* swapchain
);

void lise_swapchain_destroy(VkDevice device, lise_swapchain* swapchain);

bool lise_swapchain_acquire_next_image_index(
	const lise_device* device,
	lise_swapchain* swapchain,
	uint64_t timeout_ns,
	VkSemaphore image_available_semaphore,
	VkFence fence,
	uint32_t* out_image_index
);

bool lise_swapchain_present(
	const lise_device* device,
	lise_swapchain* swapchain,
	VkSemaphore render_complete_semaphore,
	uint32_t present_image_index
);

lise_swapchain_info lise_swapchain_query_info(lise_device* device, VkExtent2D window_extent);
