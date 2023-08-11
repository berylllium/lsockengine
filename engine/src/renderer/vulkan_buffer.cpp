#include "renderer/vulkan_buffer.hpp"

#include <cstring>

#include <simple-logger.hpp>

#include "renderer/command_buffer.hpp"

namespace lise
{

std::unique_ptr<VulkanBuffer> VulkanBuffer::create(
	const Device* device,
	uint64_t size,
	vk::BufferUsageFlags usage,
	vk::MemoryPropertyFlags memory_property_flags,
	bool bind_on_create
)
{
	auto out = std::make_unique<VulkanBuffer>();
	
	// Copy trivial data.
	out->size = size;
	out->usage = usage;
	out->device = device;
	
	vk::BufferCreateInfo buffer_ci(
		{},
		size,
		usage,
		vk::SharingMode::eExclusive
	);

	vk::Result r;

	std::tie(r, out->handle) = device->logical_device.createBuffer(buffer_ci);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to create buffer.");

		return nullptr;
	}

	// Gather memory requirements
	auto memory_properties = out->device->physical_device_memory_properties;
	auto mem_reqs = out->device->logical_device.getBufferMemoryRequirements(out->handle);

	int32_t memory_type = -1;
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
	{
		if (mem_reqs.memoryTypeBits & (1 << i) &&
			(memory_properties.memoryTypes[i].propertyFlags & memory_property_flags) == memory_property_flags &&
			(static_cast<uint32_t>(
				memory_properties.memoryTypes[i].propertyFlags &
				vk::MemoryPropertyFlagBits::eDeviceCoherentAMD
			) == 0))
		{
			memory_type = i;
		}
	}

	if (memory_type == -1)
	{
		sl::log_error("Required memory type was not found.");
		return nullptr;
	}

	out->memory_index = memory_type;

	// Allocate memory
	vk::MemoryAllocateInfo allocate_info(
		mem_reqs.size,
		memory_type
	);

	std::tie(r, out->memory) = out->device->logical_device.allocateMemory(allocate_info);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to allocate memory for buffer");
		return nullptr;
	}

	if (bind_on_create)
	{
		out->bind(0);
	}

	return out;
}

VulkanBuffer::~VulkanBuffer()
{
	device->logical_device.freeMemory(memory);

	device->logical_device.destroyBuffer(handle);
}

bool VulkanBuffer::resize(uint64_t new_size, vk::Queue& queue, vk::CommandPool& pool)
{
	vk::BufferCreateInfo buffer_ci(
		{},
		new_size,
		usage
	);

	vk::Result r;
	vk::Buffer new_buffer;

	std::tie(r, new_buffer) = device->logical_device.createBuffer(buffer_ci);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to create buffer.");
		return false;
	}

	// Gather memory requirements
	auto mem_reqs = device->logical_device.getBufferMemoryRequirements(new_buffer);

	// Allocate memory
	vk::MemoryAllocateInfo allocate_info(
		mem_reqs.size,
		memory_index
	);

	vk::DeviceMemory new_memory;
	std::tie(r, new_memory) = device->logical_device.allocateMemory(allocate_info);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to allocate memory for buffer.");
		return false;
	}

	// Bind new memory
	if (device->logical_device.bindBufferMemory(new_buffer, new_memory, 0) != vk::Result::eSuccess)
	{
		sl::log_error("Failed to bind memory to buffer.");
		return false;
	}

	// Copy data over
	copy_to(pool, nullptr, queue, 0, new_buffer, 0, size);

	// Make sure operation finished
	r = device->logical_device.waitIdle();

	// Destroy old buffer and memory
	if (memory)
	{
		device->logical_device.freeMemory(memory);
	}

	if (handle)
	{
		device->logical_device.destroyBuffer(handle);
	}

	// Set new data
	handle = new_buffer;
	memory = new_memory;
	size = new_size;

	return true;
}

bool VulkanBuffer::bind(uint64_t offset)
{
	if (device->logical_device.bindBufferMemory(handle, memory, offset) != vk::Result::eSuccess)
	{
		sl::log_error("Failed to bind buffer memory.");
		return false;
	}

	return true;
}

void* VulkanBuffer::lock_memory(uint64_t offset, uint64_t size, vk::MemoryMapFlags flags)
{
	auto [r, v] = device->logical_device.mapMemory(memory, offset, size, flags);
	
	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to map memory.");
		return nullptr;
	}

	return v;
}

void VulkanBuffer::unlock_memory()
{
	device->logical_device.unmapMemory(memory);
}

void VulkanBuffer::load_data(uint64_t offset, uint64_t size, vk::MemoryMapFlags flags, const void* data)
{
	void* buffer_data = lock_memory(offset, size, flags);

	memcpy(buffer_data, data, size);

	unlock_memory();
}

void VulkanBuffer::copy_to(
	vk::CommandPool pool,
	vk::Fence fence,
	vk::Queue queue,
	uint64_t source_offset,
	vk::Buffer dest,
	uint64_t dest_offset,
	uint64_t size
)
{
	vk::Result r = queue.waitIdle();

	if (r != vk::Result::eSuccess)
	{
		sl::log_warn("Failed to wait on queue during buffer copy.");
	}

	// Create one time use command buffer
	auto cb = CommandBuffer::create(device, pool, true);

	cb->begin(true, false, false);

	vk::BufferCopy buffer_copy(
		source_offset,
		dest_offset,
		size
	);

	cb->handle.copyBuffer(handle, dest, 1, &buffer_copy);

	cb->end_and_submit_single_use(queue);
}

}
