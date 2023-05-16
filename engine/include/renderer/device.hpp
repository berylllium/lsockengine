#pragma once

#include <memory>

#include <vulkan/vulkan.h>

#include "definitions.hpp"

namespace lise
{

struct DeviceSwapChainSupportInfo
{
	VkSurfaceCapabilitiesKHR surface_capabilities;

	uint32_t surface_format_count;
	std::unique_ptr<VkSurfaceFormatKHR[]> surface_formats;

	uint32_t present_mode_count;
	std::unique_ptr<VkPresentModeKHR[]> present_modes;
};

/**
 * @brief A structure containing queue family indices. An index of UINT32_MAX (2^32 - 1) represents
 * an unavailable queue family.
 */
struct DeviceQueueIndices
{
	uint32_t graphics_queue_index;
	uint32_t present_queue_index;
	uint32_t transfer_queue_index;
};

class Device
{
public:
	Device(
		VkInstance vulkan_instance,
		const char** physical_device_extensions,
		uint32_t physical_device_extension_count,
		const char** validation_layers,
		uint32_t validation_layer_count,
		VkSurfaceKHR surface
	);

	Device(Device&) = delete; // Prevent copies.

	~Device();

	operator VkDevice() const;
	operator VkPhysicalDevice() const;

	/**
	 * @brief Queries the devices swapchain suppot for the given surface to be used with the swapchain. 
	 * 
	 * @param surface The surface to be used with the swapchain.
	 */
	static DeviceSwapChainSupportInfo query_swapchain_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

	VkPhysicalDeviceMemoryProperties get_memory_properties() const;

	DeviceQueueIndices get_queue_indices() const;

	VkQueue get_present_queue() const;

private:
	VkPhysicalDevice physical_device;

	VkPhysicalDeviceProperties physical_device_properties;
	VkPhysicalDeviceFeatures physical_device_features;
	VkPhysicalDeviceMemoryProperties physical_device_memory_properties;

	DeviceQueueIndices queue_indices;

	VkDevice logical_device;

	VkQueue graphics_queue;
	VkQueue present_queue;
	VkQueue transfer_queue;

	VkCommandPool graphics_command_pool;

	void pick_physical_device(
		VkInstance vulkan_instance,
		const char** physical_device_extensions,
		uint32_t physical_device_extension_count,
		VkSurfaceKHR surface
	);

	static bool is_physical_device_suitable(
		VkPhysicalDevice physical_device,
		const char** physical_device_extensions,
		uint32_t physical_device_extension_count,
		VkSurfaceKHR surface
	);

	static DeviceQueueIndices find_queue_families(
		VkPhysicalDevice physical_device,
		VkSurfaceKHR surface
	);

};

}
