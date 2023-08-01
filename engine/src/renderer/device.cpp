#include "renderer/device.hpp"

#include <memory>
#include <cstring>
#include <stdexcept>

#include <simple-logger.hpp>

#define AMD_VENDOR_ID 0x1002

namespace lise
{

Device::Device(
	VkInstance vulkan_instance,
	const char** physical_device_extensions,	
	uint32_t physical_device_extension_count,
	const char** validation_layers,
	uint32_t validation_layer_count,
	VkSurfaceKHR surface
)
{
	// Pick physical device
	pick_physical_device(
		vulkan_instance,
		physical_device_extensions,
		physical_device_extension_count,
		surface
	);

	// Get info about chosen physical device
	vkGetPhysicalDeviceProperties(
		physical_device,
		&physical_device_properties
	);

	vkGetPhysicalDeviceFeatures(
		physical_device,
		&physical_device_features
	);

	vkGetPhysicalDeviceMemoryProperties(
		physical_device,
		&physical_device_memory_properties
	);

	queue_indices = find_queue_families(physical_device, surface);

	// Create the logical device
	auto unique_queues_count = uint32_t {1}; 	// We initialize to zero because the graphics queue will
										// always be present

	auto present_unique =
		queue_indices.present_queue_index != queue_indices.graphics_queue_index;

	auto transfer_unique =
		queue_indices.transfer_queue_index != queue_indices.graphics_queue_index &&
		queue_indices.transfer_queue_index != queue_indices.present_queue_index;
	
	if (present_unique) unique_queues_count++;

	if (transfer_unique) unique_queues_count++;

	auto unique_queue_indices = std::make_unique<uint32_t[]>(unique_queues_count);

	auto i = int { 0 };
	unique_queue_indices[i++] = queue_indices.graphics_queue_index;

	if (present_unique)
		unique_queue_indices[i++] = queue_indices.present_queue_index;
	
	if (transfer_unique)
		unique_queue_indices[i++] = queue_indices.transfer_queue_index;

	auto queue_create_infos = std::make_unique<VkDeviceQueueCreateInfo[]>(unique_queues_count);

	auto queue_priority = 1.0f;
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
	auto device_features = VkPhysicalDeviceFeatures {};

	// Create device
	auto create_info = VkDeviceCreateInfo {};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	create_info.queueCreateInfoCount = unique_queues_count;
	create_info.pQueueCreateInfos = queue_create_infos.get();

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

	if (vkCreateDevice(physical_device, &create_info, NULL, &logical_device)
		!= VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create the logical device.");
	}

	// Get queues
	vkGetDeviceQueue(
		logical_device,
		queue_indices.graphics_queue_index,
		0,
		&graphics_queue
	);

	vkGetDeviceQueue(
		logical_device,
		queue_indices.present_queue_index,
		0,
		&present_queue
	);

	vkGetDeviceQueue(
		logical_device,
		queue_indices.transfer_queue_index,
		0,
		&transfer_queue
	);

	// Create the graphics command pool
	auto graphics_pool_ci = VkCommandPoolCreateInfo {};
	graphics_pool_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	graphics_pool_ci.queueFamilyIndex = queue_indices.graphics_queue_index;
	graphics_pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(logical_device, &graphics_pool_ci, NULL, &graphics_command_pool) != VK_SUCCESS)
	{
		sl::log_error("Failed to create the graphics command pool.");
		throw std::exception();
	}
}

Device::~Device()
{
	// Destroy graphics command pool
	vkDestroyCommandPool(logical_device, graphics_command_pool, NULL);

	// Destroy the logical device
	if (logical_device)
	{
		vkDestroyDevice(logical_device, NULL);
		logical_device = NULL;
	}
}

Device::operator VkDevice() const
{
	return logical_device;
}

Device::operator VkPhysicalDevice() const
{
	return physical_device;
}

// Static helper functions
void Device::pick_physical_device(
	VkInstance vulkan_instance,
	const char** physical_device_extensions,
	uint32_t physical_device_extension_count,
	VkSurfaceKHR surface
)
{
	auto physical_device_count = uint32_t { 0 };
	vkEnumeratePhysicalDevices(vulkan_instance, &physical_device_count, NULL);

	if (physical_device_count == 0)
	{
		sl::log_error("Failed to find GPUs with Vulkan support.");
		throw std::exception();
	}

	auto physical_devices = std::make_unique<VkPhysicalDevice[]>(physical_device_count);

	vkEnumeratePhysicalDevices(vulkan_instance, &physical_device_count, physical_devices.get());

	for (uint32_t i = 0; i < physical_device_count; i++)
	{
		if (is_physical_device_suitable(
			physical_devices[i], 
			physical_device_extensions,
			physical_device_extension_count,
			surface
		))
		{
			physical_device = physical_devices[i];
			break;
		}
	}

	if (physical_device == VK_NULL_HANDLE)
	{
		sl::log_error("Failed to find a suitable GPU.");
		throw std::exception();
	}
}

bool Device::is_physical_device_suitable(
	VkPhysicalDevice physical_device,
	const char** physical_device_extensions,
	uint32_t physical_device_extension_count,
	VkSurfaceKHR surface
)
{
	// Get the queue family indices
	auto queue_indices = find_queue_families(physical_device, surface);

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
	
	auto available_extensions = std::make_unique<VkExtensionProperties[]>(extension_count);

	vkEnumerateDeviceExtensionProperties(physical_device, NULL, &extension_count, available_extensions.get());
	
	for (uint32_t i = 0; i < physical_device_extension_count; i++)
	{
		// Check if given device extensions are contained in available_extensions
		auto contains = false;
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
			// The device is not suitable if it does not support all the required extensions.
			return false;
		}
	}

	// Check if swapchain supported by the physcial device is adequate for our needs
	auto swap_chain_info = query_swapchain_support(physical_device, surface);

	if (swap_chain_info.surface_format_count == 0 || swap_chain_info.present_mode_count == 0)
	{
		// The device is not suitable if it does not support any presentation modes or surface
		// formats.
		return false;
	}

	return true;
}

DeviceQueueIndices Device::find_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
	auto queue_indices = DeviceQueueIndices {};
	queue_indices.graphics_queue_index = UINT32_MAX;
	queue_indices.present_queue_index = UINT32_MAX;
	queue_indices.transfer_queue_index = UINT32_MAX;

	auto queue_family_count = uint32_t { 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);

	auto queue_families = std::make_unique<VkQueueFamilyProperties[]>(queue_family_count);

	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.get());

	for (uint32_t i = 0; i < queue_family_count; i++)
	{
		if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			queue_indices.graphics_queue_index = i;
		}

		auto present_support = VkBool32 { false };
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

	return queue_indices;
}

VkPhysicalDeviceMemoryProperties Device::get_memory_properties() const
{
	return physical_device_memory_properties;
}

VkPhysicalDeviceProperties Device::get_properties() const
{
	return physical_device_properties;
}

DeviceQueueIndices Device::get_queue_indices() const
{
	return queue_indices;
}

VkQueue Device::get_graphics_queue() const
{
	return graphics_queue;
}

VkQueue Device::get_present_queue() const
{
	return present_queue;
}

VkCommandPool Device::get_graphics_command_pool() const
{
	return graphics_command_pool;
}

DeviceSwapChainSupportInfo Device::query_swapchain_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
	DeviceSwapChainSupportInfo swap_chain_info = {};

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
		swap_chain_info.surface_formats = std::make_unique<VkSurfaceFormatKHR[]>(swap_chain_info.surface_format_count);
		
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			physical_device,
			surface,
			&swap_chain_info.surface_format_count,
			swap_chain_info.surface_formats.get()
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
		swap_chain_info.present_modes = std::make_unique<VkPresentModeKHR[]>(swap_chain_info.present_mode_count);
		
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			physical_device,
			surface,
			&swap_chain_info.present_mode_count,
			swap_chain_info.present_modes.get()
		);
	}
	
	return swap_chain_info;
}

}
