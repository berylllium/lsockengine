#include "renderer/swapchain.h"

#include <stdlib.h>

#include "util/math_utils.h"
#include "core/logger.h"

bool lise_swapchain_create(
	lise_device* device,
	VkSurfaceKHR surface,
	VkExtent2D window_extent,
	lise_swapchain* out_swapchain
)
{
	out_swapchain->swapchain_out_of_date = false;
	out_swapchain->max_frames_in_flight = 2;

	lise_device_swap_chain_support_info swap_chain_support_info = device->device_swapchain_support_info;
	
	// Choose swap surface format
	VkSurfaceFormatKHR surface_format = swap_chain_support_info.surface_formats[0]; // Default, first surface format.

	for (uint32_t i = 0; i < swap_chain_support_info.surface_format_count; i++)
	{
		if (swap_chain_support_info.surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
			swap_chain_support_info.surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			surface_format = swap_chain_support_info.surface_formats[i];
			break;
		}
	}

	out_swapchain->image_format = surface_format;

	// Choose swap present mode
	VkPresentModeKHR surface_present_mode = VK_PRESENT_MODE_FIFO_KHR; // Default, guaranteed present mode

	for (uint32_t i = 0; i < swap_chain_support_info.present_mode_count; i++)
	{
		if (swap_chain_support_info.present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			surface_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
	}

	// Choose swap extent
	VkExtent2D swapchain_extent;

	if (swap_chain_support_info.surface_capabilities.currentExtent.width != UINT32_MAX)
	{
		swapchain_extent = swap_chain_support_info.surface_capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actual_extent = window_extent;

		actual_extent.width = lise_clamp(
			actual_extent.width,
			swap_chain_support_info.surface_capabilities.minImageExtent.width,
			swap_chain_support_info.surface_capabilities.maxImageExtent.width
		);

		actual_extent.height = lise_clamp(
			actual_extent.height,
			swap_chain_support_info.surface_capabilities.minImageExtent.height,
			swap_chain_support_info.surface_capabilities.maxImageExtent.height
		);

		swapchain_extent = actual_extent;
	}

	uint32_t swap_chain_image_count = swap_chain_support_info.surface_capabilities.minImageCount + 1;

	// Make sure to not exceed the maximum image count
	if (swap_chain_image_count > swap_chain_support_info.surface_capabilities.maxImageCount)
	{
		swap_chain_image_count = swap_chain_support_info.surface_capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swap_chain_ci = {};
	swap_chain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swap_chain_ci.surface = surface;
	swap_chain_ci.minImageCount = swap_chain_image_count;
	swap_chain_ci.imageFormat = surface_format.format;
	swap_chain_ci.imageColorSpace = surface_format.colorSpace;
	swap_chain_ci.imageExtent = swapchain_extent;
	swap_chain_ci.imageArrayLayers = 1;
	swap_chain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queue_indices[] = {device->queue_indices.graphics_queue_index, device->queue_indices.present_queue_index};

	if (device->queue_indices.graphics_queue_index != device->queue_indices.present_queue_index)
	{
		swap_chain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swap_chain_ci.queueFamilyIndexCount = 2;
		swap_chain_ci.pQueueFamilyIndices = queue_indices;
	}
	else
	{
		swap_chain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	swap_chain_ci.preTransform = device->device_swapchain_support_info.surface_capabilities.currentTransform;
	swap_chain_ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swap_chain_ci.presentMode = surface_present_mode;
	swap_chain_ci.clipped = VK_TRUE;
	swap_chain_ci.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device->logical_device, &swap_chain_ci, NULL, &out_swapchain->swapchain_handle)
			!= VK_SUCCESS)
	{
		LFATAL("Failed to create the swap chain.");
		return false;
	}

	// Set current frame to zero.
	out_swapchain->current_frame = 0;

	// Get swapchain images
	out_swapchain->image_count = 0;

	vkGetSwapchainImagesKHR(device->logical_device, out_swapchain->swapchain_handle, &out_swapchain->image_count, NULL);

	if (!out_swapchain->images)
	{
		out_swapchain->images = malloc(sizeof(VkImage) * out_swapchain->image_count);
	}

	vkGetSwapchainImagesKHR(
		device->logical_device,
		out_swapchain->swapchain_handle,
		&out_swapchain->image_count,
		out_swapchain->images
	);
	
	// Views
	if (!out_swapchain->image_views)
	{
		out_swapchain->image_views = malloc(sizeof(VkImageView) * out_swapchain->image_count);
	}

	for (uint32_t i = 0; i < out_swapchain->image_count; i++)
	{
		VkImageViewCreateInfo view_ci = {};
		view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_ci.image = out_swapchain->images[i];
		view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_ci.format = surface_format.format;
		view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_ci.subresourceRange.baseMipLevel = 0;
		view_ci.subresourceRange.levelCount = 1;
		view_ci.subresourceRange.baseArrayLayer = 0;
		view_ci.subresourceRange.layerCount = 1;

		vkCreateImageView(device->logical_device, &view_ci, NULL, &out_swapchain->image_views[i]);
	}

	// Check if device supports depth format
	const uint32_t candidate_count = 3;
	const VkFormat candidates[3] = {
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT
	};

	bool formats_supported = false;
	VkFormat depth_format;
	uint32_t flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
	for (uint32_t i = 0; i < candidate_count; i++)
	{
		VkFormatProperties format_properties;
		vkGetPhysicalDeviceFormatProperties(device->physical_device, candidates[i], &format_properties);

		if ((format_properties.linearTilingFeatures & flags) == flags)
		{
			depth_format = candidates[i];
			formats_supported = true;
			break;
		}
		else if ((format_properties.optimalTilingFeatures & flags) == flags)
		{
			depth_format = candidates[i];
			formats_supported = true;
			break;
		}
	}

	if (!formats_supported)
	{
		LFATAL("Failed to find supported depth format during swapchain creation.");
		return false;
	}

	out_swapchain->depth_format = depth_format;

	// Create the depth attachments
	if (!out_swapchain->depth_attachments)
	{
		out_swapchain->depth_attachments = malloc(sizeof(lise_vulkan_image) * out_swapchain->image_count);
	}

	for (uint32_t i = 0; i < out_swapchain->image_count; i++)
	{
		lise_vulkan_image_create(
			device,
			VK_IMAGE_TYPE_2D,
			swapchain_extent.width,
			swapchain_extent.height,
			depth_format,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			true,
			VK_IMAGE_ASPECT_DEPTH_BIT,
			&out_swapchain->depth_attachments[i]
		);
	}

	// TODO: Create framebuffers

	return true;
}

bool lise_swapchain_recreate(
	lise_device* device,
	VkSurfaceKHR surface,
	VkExtent2D window_extent,
	lise_swapchain* swapchain
)
{
	lise_swapchain_destroy(device->logical_device, swapchain);
	return lise_swapchain_create(device, surface, window_extent, swapchain);
}

void lise_swapchain_destroy(VkDevice device, lise_swapchain* swapchain)
{
	for (uint32_t i = 0; i < swapchain->image_count; i++)
	{
		lise_vulkan_image_destroy(device, &swapchain->depth_attachments[i]);

		vkDestroyImageView(device, swapchain->image_views[i], NULL);
	}

	free(swapchain->images);
	swapchain->images = NULL;
	free(swapchain->image_views);
	swapchain->image_views = NULL;
	free(swapchain->depth_attachments);
	swapchain->depth_attachments = NULL;

	vkDestroySwapchainKHR(device, swapchain->swapchain_handle, NULL);
	swapchain->swapchain_handle = NULL;
}

bool lise_swapchain_acquire_next_image_index(
	const lise_device* device,
	lise_swapchain* swapchain,
	uint64_t timeout_ns,
	VkSemaphore image_available_semaphore,
	VkFence fence,
	uint32_t* out_image_index
)
{
	VkResult result = vkAcquireNextImageKHR(
		device->logical_device,
		swapchain->swapchain_handle,
		timeout_ns,
		image_available_semaphore,
		fence,
		out_image_index
	);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		LDEBUG("Swapchain is out of date. Attempring to recreate.");
		swapchain->swapchain_out_of_date = true;
		return false;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		LFATAL("Failed to acquire next swapchain image.");
		return false;
	}

	return true;
}

bool lise_swapchain_present(
	const lise_device* device,
	lise_swapchain* swapchain,
	VkSemaphore render_complete_semaphore,
	uint32_t present_image_index
)
{
	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &render_complete_semaphore;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &swapchain->swapchain_handle;
	present_info.pImageIndices = &present_image_index;
	
	VkResult result = vkQueuePresentKHR(device->present_queue, &present_info);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		LDEBUG("Swapchain is out of date. Attempring to recreate.");
		swapchain->swapchain_out_of_date = true;
		return false;
	}
	else if (result != VK_SUCCESS)
	{
		LFATAL("Failed to present swapchain image.");
		return false;
	}

	swapchain->current_frame = (swapchain->current_frame + 1) % swapchain->max_frames_in_flight;

	return true;
}
