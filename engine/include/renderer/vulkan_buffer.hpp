#pragma once

#include <vulkan/vulkan.h>

#include "renderer/device.hpp"
#include "definitions.hpp"

namespace lise
{

class VulkanBuffer
{
public:
	VulkanBuffer(
		const Device& device,
		VkPhysicalDeviceMemoryProperties memory_properties,
		uint64_t size,
		VkBufferUsageFlagBits usage,
		uint32_t memory_property_flags,
		bool bind_on_create
	);

	VulkanBuffer(const VulkanBuffer&) = delete; // Prevent copies.

	~VulkanBuffer();

	operator VkBuffer() const;

	VulkanBuffer& operator = (const VulkanBuffer&) = delete; // Prevent copies.

	bool resize(uint64_t new_size, VkQueue queue, VkCommandPool pool);

	bool bind(uint64_t offset);

	void* lock_memory(uint64_t offset, uint64_t size, uint32_t flags);

	void unlock_memory();

	void load_data(uint64_t offset, uint64_t size, uint32_t flags, const void* data);

	void copy_to(
		VkCommandPool pool,
		VkFence fence,
		VkQueue queue,
		uint64_t source_offset,
		VkBuffer dest,
		uint64_t dest_offset,
		uint64_t size
	);

	VkBuffer get_handle() const;

private:
	VkBuffer handle;

	uint64_t size;

	VkBufferUsageFlagBits usage;

	bool is_locked;

	VkDeviceMemory memory;
	int32_t memory_index;
	uint32_t memory_property_flags;

	const Device& device;
};

}
