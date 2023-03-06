#include "renderer/vulkan_buffer.h"

#include <string.h>

#include "core/logger.h"
#include "renderer/command_buffer.h"

bool lise_vulkan_buffer_create(
	VkDevice device,
	VkPhysicalDeviceMemoryProperties memory_properties,
	uint64_t size,
	VkBufferUsageFlagBits usage,
	uint32_t memory_property_flags,
	bool bind_on_create,
	lise_vulkan_buffer* out_buffer
)
{
	memset(out_buffer, 0, sizeof(lise_vulkan_buffer));
	
	out_buffer->size = size;
	out_buffer->usage = usage;
	out_buffer->memory_property_flags = memory_property_flags;

	VkBufferCreateInfo buffer_ci = {};
	buffer_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_ci.size = size;
	buffer_ci.usage = usage;
	buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Only used in one queue.

	if (vkCreateBuffer(device, &buffer_ci, NULL, &out_buffer->handle) != VK_SUCCESS)
	{
		LERROR("Failed to create buffer.");
		return false;
	}

	// Gather memory requirements
	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(device, out_buffer->handle, &mem_reqs);

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
		LERROR("Required memory type was not found.");
		return false;
	}

	out_buffer->memory_index = memory_type;

	// Allocate memory
	VkMemoryAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocate_info.allocationSize = mem_reqs.size;
	allocate_info.memoryTypeIndex = memory_type;

	if (vkAllocateMemory(device, &allocate_info, NULL, &out_buffer->memory) != VK_SUCCESS)
	{
		LERROR("Failed to allocate memory for buffer");
		return false;
	}

	if (bind_on_create)
	{
		return lise_vulkan_buffer_bind(device, out_buffer, 0);
	}

	return true;
}

void lise_vulkan_buffer_destroy(VkDevice device, lise_vulkan_buffer* buffer)
{
	if (buffer->memory)
	{
		vkFreeMemory(device, buffer->memory, NULL);
		buffer->memory = NULL;
	}

	if (buffer->handle)
	{
		vkDestroyBuffer(device, buffer->handle, NULL);
		buffer->handle = NULL;
	}

	buffer->size = 0;
	buffer->usage = 0;
	buffer->is_locked = false;
}

bool lise_vulkan_buffer_resize(
	VkDevice device,
	uint64_t new_size,
	lise_vulkan_buffer* buffer,
	VkQueue queue,
	VkCommandPool pool
)
{
	VkBufferCreateInfo buffer_ci = {};
	buffer_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_ci.size = new_size;
	buffer_ci.usage = buffer->usage;
	buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Only used in one queue.
	
	VkBuffer new_buffer;
	if (vkCreateBuffer(device, &buffer_ci, NULL, &new_buffer) != VK_SUCCESS)
	{
		LERROR("Failed to create buffer.");
		return false;
	}

	// Gather memory requirements
	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(device, new_buffer, &mem_reqs);

	// Allocate memory
	VkMemoryAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocate_info.allocationSize = mem_reqs.size;
	allocate_info.memoryTypeIndex = buffer->memory_index;

	VkDeviceMemory new_memory;
	if (vkAllocateMemory(device, &allocate_info, NULL, &new_memory) != VK_SUCCESS)
	{
		LERROR("Failed to allocate memory for buffer.");
		return false;
	}

	// Bind new memory
	if (vkBindBufferMemory(device, new_buffer, new_memory, 0) != VK_SUCCESS)
	{
		LERROR("Failed to bind memory to buffer.");
		return false;
	}

	// Copy data over
	lise_vulkan_buffer_copy_to(device, pool, NULL, queue, buffer->handle, 0, new_buffer, 0, buffer->size);

	// Make sure operation finished
	vkDeviceWaitIdle(device);

	// Destroy old buffer and memory
	if (buffer->memory)
	{
		vkFreeMemory(device, buffer->memory, NULL);
		buffer->memory = NULL;
	}

	if (buffer->handle)
	{
		vkDestroyBuffer(device, buffer->handle, NULL);
		buffer->handle = NULL;
	}

	// Set new data
	buffer->handle = new_buffer;
	buffer->memory = new_memory;
	buffer->size = new_size;

	return true;
}

bool lise_vulkan_buffer_bind(VkDevice device, lise_vulkan_buffer* buffer, uint64_t offset)
{
	if (vkBindBufferMemory(device, buffer->handle, buffer->memory, offset) != VK_SUCCESS)
	{
		LERROR("Failed to bind buffer memory.");
		return false;
	}

	return true;
}

void* lise_vulkan_buffer_lock_memory(
	VkDevice device, 
	lise_vulkan_buffer* buffer,
	uint64_t offset,
	uint64_t size,
	uint32_t flags
)
{
	void* data;

	// TODO: Add error handling
	vkMapMemory(device, buffer->memory, offset, size, flags, &data);

	return data;
}

void lise_vulkan_buffer_unlock_memory(VkDevice device, lise_vulkan_buffer* buffer)
{
	vkUnmapMemory(device, buffer->memory);
}

void lise_vulkan_buffer_load_data(
	VkDevice device,
	lise_vulkan_buffer* buffer,
	uint64_t offset,
	uint64_t size,
	uint32_t flags,
	const void* data
)
{
	void* buffer_data = lise_vulkan_buffer_lock_memory(device, buffer, offset, size, flags);

	memcpy(buffer_data, data, size);

	lise_vulkan_buffer_unlock_memory(device, buffer);
}

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
)
{
	vkQueueWaitIdle(queue);

	// Create one time use command buffer
	lise_command_buffer cb;
	lise_command_buffer_allocate_and_begin_single_use(device, pool, &cb);

	VkBufferCopy buffer_copy = {};
	buffer_copy.srcOffset = source_offset;
	buffer_copy.dstOffset = dest_offset;
	buffer_copy.size = size;

	vkCmdCopyBuffer(cb.handle, source, dest, 1, &buffer_copy);

	lise_command_buffer_end_and_submit_single_use(device, pool, &cb, queue);
}
