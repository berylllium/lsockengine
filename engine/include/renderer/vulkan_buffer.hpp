#pragma once

#include <vulkan/vulkan.h>

#include "renderer/device.hpp"
#include "definitions.hpp"

namespace lise
{

class VulkanBuffer
{
public:
	vk::Buffer handle;

	uint64_t size;

	vk::BufferUsageFlags usage;

	bool is_locked;

	vk::DeviceMemory memory;
	int32_t memory_index;
	uint32_t memory_property_flags;

	const Device* device;

	VulkanBuffer() = default;

	VulkanBuffer(const VulkanBuffer&) = delete; // Prevent copies.

	~VulkanBuffer();

	VulkanBuffer& operator = (const VulkanBuffer&) = delete; // Prevent copies.
	
	LAPI static std::unique_ptr<VulkanBuffer> create(
		const Device* device,
		uint64_t size,
		vk::BufferUsageFlags usage,
		vk::MemoryPropertyFlags memory_property_flags,
		bool bind_on_create
	);

	bool resize(uint64_t new_size, vk::Queue& queue, vk::CommandPool& pool);

	bool bind(uint64_t offset);

	void* lock_memory(uint64_t offset, uint64_t size, vk::MemoryMapFlags flags);

	void unlock_memory();

	void load_data(uint64_t offset, uint64_t size, vk::MemoryMapFlags flags, const void* data);

	void copy_to(
		vk::CommandPool pool,
		vk::Fence fence,
		vk::Queue queue,
		uint64_t source_offset,
		vk::Buffer dest,
		uint64_t dest_offset,
		uint64_t size
	);

private:
};

}
