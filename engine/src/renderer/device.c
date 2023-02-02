#include "renderer/device.h"

#include <stdlib.h>
#include <string.h>

#include "core/logger.h"
#include "util/string_utils.h"

static bool pick_physical_device(
	VkInstance vulkan_instance,
	const char** physical_device_extensions,
	uint32_t physical_device_extension_count,
	VkSurfaceKHR surface,
	VkPhysicalDevice* out_physical_device
);

static bool is_physical_device_suitable(
	VkPhysicalDevice physical_device,
	const char** physical_device_extensions,
	uint32_t physical_device_extension_count,
	VkSurfaceKHR surface
);

static lise_device_queue_indices find_queue_families(
	VkPhysicalDevice physical_device,
	VkSurfaceKHR surface
);

static lise_device_swap_chain_support_info query_swap_chain_support(
	VkPhysicalDevice physical_device,
	VkSurfaceKHR surface
);

lise_device lise_device_create()
{

}

void lise_device_destroy(lise_device* device)
{

}

void lise_device_destroy_swap_chain_support_info(lise_device_swap_chain_support_info* swap_info)
{
	free(swap_info->surface_formats);
	free(swap_info->present_modes);

	memset(swap_info, 0, sizeof(lise_device_swap_chain_support_info));
}

// Static helper functions
static bool pick_physical_device(
	VkInstance vulkan_instance,
	const char** physical_device_extensions,
	uint32_t physical_device_extension_count,
	VkSurfaceKHR surface,
	VkPhysicalDevice* out_physical_device
)
{
	*out_physical_device = VK_NULL_HANDLE;

	uint32_t physical_device_count = 0;
	vkEnumeratePhysicalDevices(vulkan_instance, &physical_device_count, NULL);

	if (physical_device_count == 0)
	{
		LFATAL("Failed to find GPUs with Vulkan support.");
		return false;
	}

	VkPhysicalDevice* physical_devices =
		malloc(physical_device_count * sizeof(VkPhysicalDevice));

	vkEnumeratePhysicalDevices(vulkan_instance, &physical_device_count, physical_devices);

	for (uint32_t i = 0; i < physical_device_count; i++)
	{
		if (is_physical_device_suitable(
			physical_devices[i], 
			physical_device_extensions,
			physical_device_extension_count,
			surface))
		{
			*out_physical_device = physical_devices[i];
			break;
		}
	}

	free(physical_devices);

	if (*out_physical_device == VK_NULL_HANDLE)
	{
		LFATAL("Failed to find a suitable GPU");
		return false;
	}

	return true;
}

static bool is_physical_device_suitable(
	VkPhysicalDevice physical_device,
	const char** physical_device_extensions,
	uint32_t physical_device_extension_count,
	VkSurfaceKHR surface
)
{
	// Get the queue family indices
	lise_device_queue_indices queue_indices = find_queue_families(physical_device, surface);

	if (queue_indices.graphics_queue_index == UINT32_MAX ||
		queue_indices.present_queue_index == UINT32_MAX)
	{
		// The given device is not suitable if it does not have a graphics or present queue.
		return false;
	}

	// Check if the required extensions are supported by the gpu
	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(physical_device, NULL, &extension_count, NULL);
	
	VkExtensionProperties* available_extensions = 
		malloc(extension_count * sizeof(VkExtensionProperties));

	vkEnumerateDeviceExtensionProperties(physical_device, NULL, &extension_count,
		available_extensions);
	
	for (uint32_t i = 0; i < extension_count; i++)
	{
		if (!lise_util_string_array_contains(
				physical_device_extensions,
				physical_device_extension_count,
				available_extensions[i].extensionName))
		{
			free(available_extensions);
			// The device is not suitable if it does not support all the required extensions.
			return false;
		}
	}

	// We will no longer need the available extensions.
	free(available_extensions);

	// Check if swapchain supported by the physcial device is adequate for our needs
	lise_device_swap_chain_support_info swap_chain_info =
		query_swap_chain_support(physical_device, surface);

	if (swap_chain_info.surface_format_count == 0 || swap_chain_info.present_mode_count == 0)
	{
		lise_device_destroy_swap_chain_support_info(&swap_chain_info);
		// The device is not suitable if it does not support any presentation modes or surface
		// formats.
		return false;
	}

	lise_device_destroy_swap_chain_support_info(&swap_chain_info);
	return true;
}

static lise_device_queue_indices find_queue_families(
	VkPhysicalDevice physical_device,
	VkSurfaceKHR surface
)
{
	lise_device_queue_indices queue_indices = {};
	queue_indices.graphics_queue_index = UINT32_MAX;
	queue_indices.present_queue_index = UINT32_MAX;
	queue_indices.transfer_queue_index = UINT32_MAX;

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);

	VkQueueFamilyProperties* queue_families =
		malloc(queue_family_count * sizeof(VkQueueFamilyProperties));

	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);

	for (uint32_t i = 0; i < queue_family_count; i++)
	{
		if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			queue_indices.graphics_queue_index = i;
		}

		VkBool32 present_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
		if (present_support)
		{
			queue_indices.present_queue_index = i;
		}

		if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			queue_indices.transfer_queue_index = i;
		}
	}

	free(queue_families);
	return queue_indices;
}

static lise_device_swap_chain_support_info query_swap_chain_support(
	VkPhysicalDevice physical_device,
	VkSurfaceKHR surface
)
{
	lise_device_swap_chain_support_info swap_chain_info = {};

	// Query basic capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		physical_device,
		surface,
		&swap_chain_info.surface_capabilities
	);
	
	// Query supported surface formats
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		physical_device,
		surface,
		&swap_chain_info.surface_format_count,
		NULL
	);
	
	if (swap_chain_info.surface_format_count > 0)
	{
		swap_chain_info.surface_formats =
			malloc(swap_chain_info.surface_format_count * sizeof(VkSurfaceFormatKHR));
		
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			physical_device,
			surface,
			&swap_chain_info.surface_format_count,
			swap_chain_info.surface_formats
		);
	}

	// Query supported presentation modes
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		physical_device,
		surface,
		&swap_chain_info.present_mode_count,
		NULL
	);

	if (swap_chain_info.present_mode_count > 0)
	{
		swap_chain_info.present_modes = 
			malloc(swap_chain_info.present_mode_count * sizeof(VkPresentModeKHR));
		
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			physical_device,
			surface,
			&swap_chain_info.present_mode_count,
			swap_chain_info.present_modes
		);
	}
	
	return swap_chain_info;
}
