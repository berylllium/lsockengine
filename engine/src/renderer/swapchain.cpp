#include "renderer/swapchain.hpp"

//#include <stdlib.h>

//#include "util/math_utils.h"
#include "core/logger.hpp"

namespace lise
{

Swapchain::Swapchain(
	const Device& device,
	VkSurfaceKHR surface,
	SwapchainInfo swapchain_info,
	const RenderPass& render_pass
) : swapchain_info(swapchain_info), device(device), surface(surface),
	swapchain_out_of_date(false), current_frame(0), image_count(0)
{
	VkSwapchainCreateInfoKHR swap_chain_ci = {};
	swap_chain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swap_chain_ci.surface = surface;
	swap_chain_ci.minImageCount = swapchain_info.min_image_count;
	swap_chain_ci.imageFormat = swapchain_info.image_format.format;
	swap_chain_ci.imageColorSpace = swapchain_info.image_format.colorSpace;
	swap_chain_ci.imageExtent = swapchain_info.swapchain_extent;
	swap_chain_ci.imageArrayLayers = 1;
	swap_chain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	DeviceQueueIndices device_queue_indices = device.get_queue_indices();

	uint32_t queue_indices[] = { device_queue_indices.graphics_queue_index, device_queue_indices.present_queue_index };

	if (device_queue_indices.graphics_queue_index != device_queue_indices.present_queue_index)
	{
		swap_chain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swap_chain_ci.queueFamilyIndexCount = 2;
		swap_chain_ci.pQueueFamilyIndices = queue_indices;
	}
	else
	{
		swap_chain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	DeviceSwapChainSupportInfo device_swap_chain_support_info = Device::query_swapchain_support(device, surface);

	swap_chain_ci.preTransform = device_swap_chain_support_info.surface_capabilities.currentTransform;
	swap_chain_ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swap_chain_ci.presentMode = swapchain_info.present_mode;
	swap_chain_ci.clipped = VK_TRUE;
	swap_chain_ci.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &swap_chain_ci, NULL, &handle)	!= VK_SUCCESS)
	{
		LFATAL("Failed to create the swap chain.");
		throw std::exception();
	}

	// Get swapchain images
	vkGetSwapchainImagesKHR(device, handle, &image_count, NULL);

	images = new VkImage[image_count];

	max_frames_in_flight = image_count - 1;

	vkGetSwapchainImagesKHR(device, handle,	&image_count, images);
	
	// Views
	image_views = new VkImageView[image_count];

	for (uint32_t i = 0; i < image_count; i++)
	{
		VkImageViewCreateInfo view_ci = {};
		view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_ci.image = images[i];
		view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_ci.format = swapchain_info.image_format.format;
		view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_ci.subresourceRange.baseMipLevel = 0;
		view_ci.subresourceRange.levelCount = 1;
		view_ci.subresourceRange.baseArrayLayer = 0;
		view_ci.subresourceRange.layerCount = 1;

		vkCreateImageView(device, &view_ci, NULL, &image_views[i]);
	}

	// Create the depth attachments
	depth_attachments.reserve(image_count);

	for (uint32_t i = 0; i < image_count; i++)
	{
		try
		{
			depth_attachments.emplace_back(
				device,
				VK_IMAGE_TYPE_2D,
				vector2ui { swapchain_info.swapchain_extent.width, swapchain_info.swapchain_extent.height },
				swapchain_info.depth_format,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				true,
				VK_IMAGE_ASPECT_DEPTH_BIT
			);
		}
		catch (std::exception e)
		{
			LFATAL("Failed to create image swap chain depth attachments.");
			throw std::exception();
		}
	}

	// Create framebuffers
	framebuffers.reserve(image_count);

	for (uint32_t i = 0; i < image_count; i++)
	{
		uint32_t attachment_count = 2;
		VkImageView attachments[] = {
			image_views[i],
			depth_attachments[i].get_image_view()
		};

		try
		{
			framebuffers.emplace_back(
				device,
				render_pass,
				vector2ui { swapchain_info.swapchain_extent.width, swapchain_info.swapchain_extent.height },
				attachment_count,
				attachments
			);
		}
		catch (std::exception e)
		{
			LFATAL("Failed to create swap chain frame buffers");
			throw std::exception();
		}
	}
}

Swapchain::~Swapchain()
{
	for (uint32_t i = 0; i < image_count; i++)
	{
		vkDestroyImageView(device, image_views[i], NULL);
	}

	delete images;

	vkDestroySwapchainKHR(device, handle, NULL);
}

bool Swapchain::acquire_next_image_index(
	uint64_t timeout_ns,
	VkSemaphore image_available_semaphore,
	VkFence fence,
	uint32_t& out_image_index
)
{
	VkResult result = vkAcquireNextImageKHR(
		device,
		handle,
		timeout_ns,
		image_available_semaphore,
		fence,
		&out_image_index
	);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		LDEBUG("Swapchain is out of date. Attempring to recreate.");
		swapchain_out_of_date = true;
		return false;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		LFATAL("Failed to acquire next swapchain image.");
		return false;
	}

	return true;
}

bool Swapchain::swapchain_present(
	VkSemaphore render_complete_semaphore,
	uint32_t present_image_index
)
{
	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &render_complete_semaphore;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &handle;
	present_info.pImageIndices = &present_image_index;
	
	VkResult result = vkQueuePresentKHR(device.get_present_queue(), &present_info);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		LDEBUG("Swapchain is out of date. Attempring to recreate.");
		swapchain_out_of_date = true;
		return false;
	}
	else if (result != VK_SUCCESS)
	{
		LFATAL("Failed to present swapchain image.");
		return false;
	}

	current_frame = (current_frame + 1) % max_frames_in_flight;

	return true;
}

SwapchainInfo Swapchain::query_info(const Device& device, VkSurfaceKHR surface)
{
	SwapchainInfo info = {};

	DeviceSwapChainSupportInfo swap_chain_support_info = Device::query_swapchain_support(device, surface);
	
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

	info.image_format = surface_format;

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

	info.present_mode = surface_present_mode;

	// Choose swap extent
	info.swapchain_extent = swap_chain_support_info.surface_capabilities.currentExtent;

	uint32_t swap_chain_image_count = swap_chain_support_info.surface_capabilities.minImageCount + 1;

	// Make sure to not exceed the maximum image count
	if (swap_chain_image_count > swap_chain_support_info.surface_capabilities.maxImageCount &&
		swap_chain_support_info.surface_capabilities.maxImageCount > 0)
	{
		swap_chain_image_count = swap_chain_support_info.surface_capabilities.maxImageCount;
	}

	info.min_image_count = swap_chain_image_count;

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
		vkGetPhysicalDeviceFormatProperties(device, candidates[i], &format_properties);

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
		return SwapchainInfo {};
	}

	info.depth_format = depth_format;

	return info;
}

}
