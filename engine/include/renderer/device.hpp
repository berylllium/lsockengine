#pragma once

#include <span>
#include <memory>

#include <vulkan/vulkan.hpp>

#include "definitions.hpp"

namespace lise
{

struct DeviceSwapChainSupportInfo
{
	vk::SurfaceCapabilitiesKHR surface_capabilities;

	std::vector<vk::SurfaceFormatKHR> surface_formats;

	std::vector<vk::PresentModeKHR> present_modes;
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


struct Device
{
	vk::PhysicalDevice physical_device;

	vk::PhysicalDeviceProperties physical_device_properties;
	vk::PhysicalDeviceFeatures physical_device_features;
	vk::PhysicalDeviceMemoryProperties physical_device_memory_properties;

	DeviceQueueIndices queue_indices;

	vk::Device logical_device;

	vk::Queue graphics_queue;
	vk::Queue present_queue;
	vk::Queue transfer_queue;

	vk::CommandPool graphics_command_pool;

	Device() = default;

	Device(Device&) = delete; // Prevent copies.
	
	~Device();

	Device& operator = (Device&) = delete;

	static std::unique_ptr<Device> create(
		vk::Instance instance,
		std::span<std::string> physical_device_extensions,
		std::span<std::string> validation_layers,
		vk::SurfaceKHR surface
	);

	/**
	 * @brief Queries the devices swapchain suppot for the given surface to be used with the swapchain. 
	 * 
	 * @param surface The surface to be used with the swapchain.
	 */
	static DeviceSwapChainSupportInfo query_swapchain_support(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);

private:
	bool pick_physical_device(
		vk::Instance& instance,
		std::span<std::string> physical_device_extensions,
		vk::SurfaceKHR& surface
	);

	static bool is_physical_device_suitable(
		vk::PhysicalDevice& physical_device,
		std::span<std::string> physical_device_extensions,
		vk::SurfaceKHR& surface
	);

	static DeviceQueueIndices find_queue_families(
		vk::PhysicalDevice& physical_device,
		vk::SurfaceKHR& surface
	);
};

}
