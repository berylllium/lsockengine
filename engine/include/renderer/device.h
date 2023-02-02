#pragma once

#include <vulkan/vulkan.h>

#include "definitions.h"

typedef struct lise_device_swap_chain_support_info
{
	VkSurfaceCapabilitiesKHR surface_capabilities;

	uint32_t surface_format_count;
	VkSurfaceFormatKHR* surface_formats;

	uint32_t present_mode_count;
	VkPresentModeKHR* present_modes;
} lise_device_swap_chain_support_info;

/**
 * @brief A structure containing queue family indices. An index of UINT32_MAX (2^32 - 1) represents
 * an unavailable queue family.
 */
typedef struct lise_device_queue_indices
{
	uint32_t graphics_queue_index;
	uint32_t present_queue_index;
	uint32_t transfer_queue_index;
} lise_device_queue_indices;

typedef struct lise_device
{
	VkPhysicalDevice physical_device;
	VkPhysicalDeviceProperties physical_device_properties;
	VkPhysicalDeviceFeatures physcial_device_features;
	VkPhysicalDeviceMemoryProperties physical_device_memory_properties;

	VkDevice logical_device;

	lise_device_swap_chain_support_info device_swapchain_support_info;

	lise_device_queue_indices queue_indices;
} lise_device;

lise_device lise_device_create();

void lise_device_destroy(lise_device* device);

void lise_device_destroy_swap_chain_support_info(lise_device_swap_chain_support_info* swap_info);
