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

bool lise_device_create(
	VkInstance vulkan_instance,
	const char** physical_device_extensions,	
	uint32_t physical_device_extension_count,
	const char** validation_layers,
	uint32_t validation_layer_count,
	VkSurfaceKHR surface,
	lise_device* out_device
)
{
	// Pick physical device
	if (!pick_physical_device(
			vulkan_instance,
			physical_device_extensions,
			physical_device_extension_count,
			surface,
			&out_device->physical_device))
	{
		LFATAL("Failed to pick a suitable physical device.");
		return false;
	}

	// Get info about chosen physical device
	vkGetPhysicalDeviceProperties(
		out_device->physical_device,
		&out_device->physical_device_properties
	);

	vkGetPhysicalDeviceFeatures(
		out_device->physical_device,
		&out_device->physical_device_features
	);

	vkGetPhysicalDeviceMemoryProperties(
		out_device->physical_device,
		&out_device->physical_device_memory_properties
	);

	out_device->device_swapchain_support_info =
		lise_device_query_swap_chain_support(out_device->physical_device, surface);
	
	out_device->queue_indices = find_queue_families(out_device->physical_device, surface);

	// Create the logical device
	uint32_t unique_queues_count = 1; 	// We initialize to zero because the graphics queue will
										// always be present

	bool present_unique =
		out_device->queue_indices.present_queue_index != out_device->queue_indices.graphics_queue_index;

	bool transfer_unique =
		out_device->queue_indices.transfer_queue_index != out_device->queue_indices.graphics_queue_index &&
		out_device->queue_indices.transfer_queue_index != out_device->queue_indices.present_queue_index;
	
	if (present_unique) unique_queues_count++;

	if (transfer_unique) unique_queues_count++;

	uint32_t* unique_queue_indices = malloc(unique_queues_count * sizeof(uint32_t));

	uint32_t i = 0;
	unique_queue_indices[i++] = out_device->queue_indices.graphics_queue_index;

	if (present_unique)
		unique_queue_indices[i++] = out_device->queue_indices.present_queue_index;
	
	if (transfer_unique)
		unique_queue_indices[i++] = out_device->queue_indices.transfer_queue_index;

	VkDeviceQueueCreateInfo* queue_create_infos =
		malloc(unique_queues_count * sizeof(VkDeviceQueueCreateInfo));

	float queue_priority = 1.0f;
	for (uint32_t i = 0; i < unique_queues_count; i++)
	{
		queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_infos[i].queueFamilyIndex = unique_queue_indices[i];
		queue_create_infos[i].queueCount = 1;
		queue_create_infos[i].pQueuePriorities = &queue_priority;

		queue_create_infos[i].flags = 0;
		queue_create_infos[i].pNext = NULL;
	}

	// Request features
	VkPhysicalDeviceFeatures device_features = {};

	// Create device
	VkDeviceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	create_info.queueCreateInfoCount = unique_queues_count;
	create_info.pQueueCreateInfos = queue_create_infos;

	create_info.pEnabledFeatures = &device_features;

	// Enable device extensions
	create_info.enabledExtensionCount = physical_device_extension_count;
	create_info.ppEnabledExtensionNames = physical_device_extensions;

#ifdef NDEBUG
	create_info.enabledLayerCount = 0;
#else
	create_info.enabledLayerCount = validation_layer_count;
	create_info.ppEnabledLayerNames = validation_layers;
#endif

	if (vkCreateDevice(out_device->physical_device, &create_info, NULL, &out_device->logical_device)
		!= VK_SUCCESS)
	{
		LFATAL("Failed to create the logical device.");

		free(unique_queue_indices);
		free(queue_create_infos);

		return false;
	}

	free(unique_queue_indices);
	free(queue_create_infos);

	// Get queues

	vkGetDeviceQueue(
		out_device->logical_device,
		out_device->queue_indices.graphics_queue_index,
		0,
		&out_device->graphics_queue
	);

	vkGetDeviceQueue(
		out_device->logical_device,
		out_device->queue_indices.present_queue_index,
		0,
		&out_device->present_queue
	);

	vkGetDeviceQueue(
		out_device->logical_device,
		out_device->queue_indices.transfer_queue_index,
		0,
		&out_device->transfer_queue
	);

	return true;	
}

void lise_device_destroy(lise_device* device)
{
	// Destroy the logical device
	if (device->logical_device)
	{
		vkDestroyDevice(device->logical_device, NULL);
		device->logical_device = NULL;
	}

	lise_device_destroy_swap_chain_support_info(&device->device_swapchain_support_info);

	memset(device, 0, sizeof(lise_device));
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
		LFATAL("Failed to find a suitable GPU.");
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
		queue_indices.present_queue_index == UINT32_MAX ||
		queue_indices.transfer_queue_index == UINT32_MAX)
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
	
	for (uint32_t i = 0; i < physical_device_extension_count; i++)
	{
		// Check if given device extensions are contained in available_extensions
		bool contains = false;
		for (uint32_t j = 9; j < extension_count; j++)
		{
			if (strcmp(physical_device_extensions[i], available_extensions[j].extensionName) == 0)
			{
				contains = true;
				break;
			}
		}

		if (!contains)
		{
			free (available_extensions);

			// The device is not suitable if it does not support all the required extensions.
			return false;
		}
	}

	// We will no longer need the available extensions.
	free(available_extensions);

	// Check if swapchain supported by the physcial device is adequate for our needs
	lise_device_swap_chain_support_info swap_chain_info =
		lise_device_query_swap_chain_support(physical_device, surface);

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

lise_device_swap_chain_support_info lise_device_query_swap_chain_support(
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
