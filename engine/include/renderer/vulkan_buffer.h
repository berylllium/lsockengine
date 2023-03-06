#pragma once

#include <vulkan/vulkan.h>

#include "definitions.h"

typedef struct lise_vulkan_buffer
{
	uint64_t size;
	VkBuffer handle;
	VkBufferUsageFlagBits usage;
	bool is_locked;
	VkDeviceMemory memory;
	int32_t memory_index;
	uint32_t memory_property_flags;
} lise_vulkan_buffer;

bool lise_vulkan_buffer_create(
	VkDevice device,
	VkPhysicalDeviceMemoryProperties memory_properties,
	uint64_t size,
	VkBufferUsageFlagBits usage,
	uint32_t memory_property_flags,
	bool bind_on_create,
	lise_vulkan_buffer* out_buffer
);

void lise_vulkan_buffer_destroy(VkDevice device, lise_vulkan_buffer* buffer);

bool lise_vulkan_buffer_resize(
	VkDevice device,
	uint64_t new_size,
	lise_vulkan_buffer* buffer,
	VkQueue queue,
	VkCommandPool pool
);

bool lise_vulkan_buffer_bind(VkDevice device, lise_vulkan_buffer* buffer, uint64_t offset);

void* lise_vulkan_buffer_lock_memory(
	VkDevice device, 
	lise_vulkan_buffer* buffer,
	uint64_t offset,
	uint64_t size,
	uint32_t flags
);

void lise_vulkan_buffer_unlock_memory(VkDevice device, lise_vulkan_buffer* buffer);

void lise_vulkan_buffer_load_data(
	VkDevice device,
	lise_vulkan_buffer* buffer,
	uint64_t offset,
	uint64_t size,
	uint32_t flags,
	const void* data
);

void lise_vulkan_buffer_copy_to(
	VkDevice device,
	VkCommandPool pool,
	VkFence fence,
	VkQueue queue,
	VkBuffer source,
	uint64_t source_offset,
	VkBuffer dest,
	uint64_t dest_offset,
	uint64_t size
);
