#include "renderer/vulkan_buffer.hpp"

#include <cstring>

#include <simple-logger.hpp>

#include "renderer/command_buffer.hpp"

namespace lise
{

VulkanBuffer::VulkanBuffer(
	const Device& device,
	VkPhysicalDeviceMemoryProperties memory_properties,
	uint64_t size,
	VkBufferUsageFlagBits usage,
	uint32_t memory_property_flags,
	bool bind_on_create
) : device(device), size(size), usage(usage), memory_property_flags(memory_property_flags)
{
	VkBufferCreateInfo buffer_ci = {};
	buffer_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_ci.size = size;
	buffer_ci.usage = usage;
	buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Only used in one queue.

	if (vkCreateBuffer(device, &buffer_ci, NULL, &handle) != VK_SUCCESS)
	{
		sl::log_error("Failed to create buffer.");
		throw std::exception();
	}

	// Gather memory requirements
	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(device, handle, &mem_reqs);

	int32_t memory_type = -1;
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
	{
		if (mem_reqs.memoryTypeBits & (1 << i) &&
			(memory_properties.memoryTypes[i].propertyFlags & memory_property_flags) == memory_property_flags &&
			(memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) == 0)
		{
			memory_type = i;
		}
	}

	if (memory_type == -1)
	{
		sl::log_error("Required memory type was not found.");
		throw std::exception();
	}

	memory_index = memory_type;

	// Allocate memory
	VkMemoryAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocate_info.allocationSize = mem_reqs.size;
	allocate_info.memoryTypeIndex = memory_type;

	if (vkAllocateMemory(device, &allocate_info, NULL, &memory) != VK_SUCCESS)
	{
		sl::log_error("Failed to allocate memory for buffer");
		throw std::exception();
	}

	if (bind_on_create)
	{
		bind(0);
	}
}

VulkanBuffer::~VulkanBuffer()
{
	vkFreeMemory(device, memory, NULL);

	vkDestroyBuffer(device, handle, NULL);
}

VulkanBuffer::operator VkBuffer() const
{
	return handle;
}

bool VulkanBuffer::resize(
	uint64_t new_size,
	VkQueue queue,
	VkCommandPool pool
)
{
	VkBufferCreateInfo buffer_ci = {};
	buffer_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_ci.size = new_size;
	buffer_ci.usage = usage;
	buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Only used in one queue.
	
	VkBuffer new_buffer;
	if (vkCreateBuffer(device, &buffer_ci, NULL, &new_buffer) != VK_SUCCESS)
	{
		sl::log_error("Failed to create buffer.");
		return false;
	}

	// Gather memory requirements
	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(device, new_buffer, &mem_reqs);

	// Allocate memory
	VkMemoryAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocate_info.allocationSize = mem_reqs.size;
	allocate_info.memoryTypeIndex = memory_index;

	VkDeviceMemory new_memory;
	if (vkAllocateMemory(device, &allocate_info, NULL, &new_memory) != VK_SUCCESS)
	{
		sl::log_error("Failed to allocate memory for buffer.");
		return false;
	}

	// Bind new memory
	if (vkBindBufferMemory(device, new_buffer, new_memory, 0) != VK_SUCCESS)
	{
		sl::log_error("Failed to bind memory to buffer.");
		return false;
	}

	// Copy data over
	copy_to(pool, NULL, queue, 0, new_buffer, 0, size);

	// Make sure operation finished
	vkDeviceWaitIdle(device);

	// Destroy old buffer and memory
	if (memory)
	{
		vkFreeMemory(device, memory, NULL);
	}

	if (handle)
	{
		vkDestroyBuffer(device, handle, NULL);
	}

	// Set new data
	handle = new_buffer;
	memory = new_memory;
	size = new_size;

	return true;
}

bool VulkanBuffer::bind(uint64_t offset)
{
	if (vkBindBufferMemory(device, handle, memory, offset) != VK_SUCCESS)
	{
		sl::log_error("Failed to bind buffer memory.");
		return false;
	}

	return true;
}

void* VulkanBuffer::lock_memory(uint64_t offset, uint64_t size, uint32_t flags)
{
	void* data;

	// TODO: Add error handling
	vkMapMemory(device, memory, offset, size, flags, &data);

	return data;
}

void VulkanBuffer::unlock_memory()
{
	vkUnmapMemory(device, memory);
}

void VulkanBuffer::load_data(uint64_t offset, uint64_t size, uint32_t flags, const void* data)
{
	void* buffer_data = lock_memory(offset, size, flags);

	memcpy(buffer_data, data, size);

	unlock_memory();
}

void VulkanBuffer::copy_to(
	VkCommandPool pool,
	VkFence fence,
	VkQueue queue,
	uint64_t source_offset,
	VkBuffer dest,
	uint64_t dest_offset,
	uint64_t size
)
{
	vkQueueWaitIdle(queue);

	// Create one time use command buffer
	CommandBuffer cb(device, pool, true);
	cb.begin(true, false, false);

	VkBufferCopy buffer_copy = {};
	buffer_copy.srcOffset = source_offset;
	buffer_copy.dstOffset = dest_offset;
	buffer_copy.size = size;

	vkCmdCopyBuffer(cb, handle, dest, 1, &buffer_copy);

	cb.end_and_submit_single_use(queue);
}

VkBuffer VulkanBuffer::get_handle() const
{
	return handle;
}

}
