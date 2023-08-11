#include "renderer/device.hpp"

#include <memory>
#include <cstring>
#include <stdexcept>

#include <simple-logger.hpp>

#define AMD_VENDOR_ID 0x1002

namespace lise
{

std::unique_ptr<Device> Device::create(
	vk::Instance instance,
	std::span<std::string> physical_device_extensions,
	std::span<std::string> validation_layers,
	vk::SurfaceKHR surface
)
{
	auto out = std::make_unique<Device>();

	if (!out->pick_physical_device(instance, physical_device_extensions, surface))
	{
		sl::log_error("Failed to pick a suitable physical device.");
		return nullptr;
	}

	// Get info about chosen physical device
	out->physical_device_properties = out->physical_device.getProperties();
	out->physical_device_features = out->physical_device.getFeatures();
	out->physical_device_memory_properties = out->physical_device.getMemoryProperties();

	out->queue_indices = find_queue_families(out->physical_device, surface);

	// Create the logical device
	uint32_t unique_queues_count = 1; 	// We initialize to one because the graphics queue will
										// always be present

	bool present_unique =
		out->queue_indices.present_queue_index != out->queue_indices.graphics_queue_index;

	bool transfer_unique =
		out->queue_indices.transfer_queue_index != out->queue_indices.graphics_queue_index &&
		out->queue_indices.transfer_queue_index != out->queue_indices.present_queue_index;
	
	if (present_unique) unique_queues_count++;

	if (transfer_unique) unique_queues_count++;

	std::vector<uint32_t> unique_queue_indices(unique_queues_count);

	int i = 0;
	unique_queue_indices[i++] = out->queue_indices.graphics_queue_index;

	if (present_unique)
		unique_queue_indices[i++] = out->queue_indices.present_queue_index;
	
	if (transfer_unique)
		unique_queue_indices[i++] = out->queue_indices.transfer_queue_index;

	std::vector<vk::DeviceQueueCreateInfo> queue_create_infos(unique_queues_count);

	float queue_priority = 1.0f;
	for (size_t i = 0; i < queue_create_infos.size(); i++)
	{
		queue_create_infos[i] = vk::DeviceQueueCreateInfo(
			{},
			unique_queue_indices[i],
			1,
			&queue_priority
		);
	}

	// Request features
	vk::PhysicalDeviceFeatures device_features = {};

	// Create device
	// Convert string array to char* array.
	std::vector<const char*> pde_chars;
	pde_chars.reserve(physical_device_extensions.size());;
	
	for (auto& s : physical_device_extensions)
	{
		pde_chars.push_back(s.c_str());
	}

	vk::DeviceCreateInfo create_info(
		{},
		queue_create_infos,
#ifdef NDEBUG
		validation_layers,
#else
		nullptr,
#endif
		pde_chars,
		&device_features
	);

	vk::Result r;

	std::tie(r, out->logical_device) = out->physical_device.createDevice(create_info);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to create logical device.");

		return nullptr;
	}

	// Get queues
	out->graphics_queue = out->logical_device.getQueue(out->queue_indices.graphics_queue_index, 0);
	out->present_queue = out->logical_device.getQueue(out->queue_indices.present_queue_index, 0);
	out->transfer_queue = out->logical_device.getQueue(out->queue_indices.transfer_queue_index, 0);
	
	// Create the graphics command pool
	vk::CommandPoolCreateInfo graphics_pool_ci(
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		out->queue_indices.graphics_queue_index
	);

	std::tie(r, out->graphics_command_pool) = out->logical_device.createCommandPool(graphics_pool_ci);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to create the graphics command pool.");

		return nullptr;
	}

	return out;
}

DeviceSwapChainSupportInfo Device::query_swapchain_support(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface)
{
	DeviceSwapChainSupportInfo swapchain_info = {};

	vk::Result r;

	// Query basic capabilities
	std::tie(r, swapchain_info.surface_capabilities) = physical_device.getSurfaceCapabilitiesKHR(surface);
	
	// Query supported surface formats
	std::tie(r, swapchain_info.surface_formats) = physical_device.getSurfaceFormatsKHR(surface);

	// Query supported presentation modes
	std::tie(r, swapchain_info.present_modes) = physical_device.getSurfacePresentModesKHR(surface);
	
	return swapchain_info;
}

Device::~Device()
{
	// Destroy graphics command pool
	logical_device.destroy(graphics_command_pool);

	// Destroy the logical device
	logical_device.destroy();
}

// Static helper functions
bool Device::pick_physical_device(
	vk::Instance& instance,
	std::span<std::string> physical_device_extensions,
	vk::SurfaceKHR& surface
)
{

	auto [enumeration_result, physical_devices] = instance.enumeratePhysicalDevices();
	
	if (enumeration_result != vk::Result::eSuccess || physical_devices.size() == 0)
	{
		sl::log_error("Failed to find GPUs with Vulkan support.");
		
		return false;
	}

	bool found_fdevice = false;
	for (auto& physical_device : physical_devices)
	{
		if (is_physical_device_suitable(physical_device, physical_device_extensions, surface))
		{
			this->physical_device = physical_device;

			found_fdevice = true;
			break;
		}
	}

	if (!found_fdevice)
	{
		sl::log_error("Failed to find a suitable GPU.");
		return false;
	}

	return true;
}

bool Device::is_physical_device_suitable(
	vk::PhysicalDevice& physical_device,
	std::span<std::string> physical_device_extensions,
	vk::SurfaceKHR& surface
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
	auto [enumerate_result, available_extensions] = physical_device.enumerateDeviceExtensionProperties();

	if (enumerate_result != vk::Result::eSuccess || available_extensions.size() == 0)
	{
		sl::log_error("Failed to enumerate physical device extension properties.");

		return false;
	}
	
	for (auto& extension : physical_device_extensions)
	{
		// Check if given device extensions are contained in available_extensions
		bool contains = false;
		for (auto& available_extension : available_extensions)
		{
			if (extension == available_extension.extensionName)
			{
				return true;
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

	if (swap_chain_info.surface_formats.size() == 0 || swap_chain_info.present_modes.size() == 0)
	{
		// The device is not suitable if it does not support any presentation modes or surface
		// formats.
		return false;
	}

	return true;
}

DeviceQueueIndices Device::find_queue_families(vk::PhysicalDevice& physical_device, vk::SurfaceKHR& surface)
{
	DeviceQueueIndices queue_indices = {};
	queue_indices.graphics_queue_index = UINT32_MAX;
	queue_indices.present_queue_index = UINT32_MAX;
	queue_indices.transfer_queue_index = UINT32_MAX;

	auto queue_families = physical_device.getQueueFamilyProperties();

	for (size_t i = 0; i < queue_families.size(); i++)
	{
		if (queue_families[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			queue_indices.graphics_queue_index = i;
		}

		auto [_, present_support] = physical_device.getSurfaceSupportKHR(i, surface);

		if (present_support)
		{
			queue_indices.present_queue_index = i;
		}

		if (queue_families[i].queueFlags & vk::QueueFlagBits::eTransfer)
		{
			queue_indices.transfer_queue_index = i;
		}
	}

	return queue_indices;
}

}
