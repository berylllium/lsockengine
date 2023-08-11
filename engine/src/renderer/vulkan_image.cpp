#include "renderer/vulkan_image.hpp"

#include <stdexcept>

#include <simple-logger.hpp>

namespace lise
{

std::unique_ptr<Image> Image::create(
	const Device* device,
	vk::ImageType image_type,
	vector2ui size,
	vk::Format image_format,
	vk::ImageTiling image_tiling,
	vk::ImageUsageFlags use_flags,
	vk::MemoryPropertyFlags memory_flags,
	bool create_view,
	vk::ImageAspectFlags view_aspect_flags
)
{
	auto out = std::make_unique<Image>();

	// Copy trivial data.
	out->device = device;
	out->size = size;
	out->image_format = image_format;

	vk::ImageCreateInfo image_create_info(
		{},
		image_type,
		image_format,
		{ size.w, size.h, 1 },
		4,
		1,
		vk::SampleCountFlagBits::e1,
		image_tiling,
		use_flags,
		vk::SharingMode::eExclusive,
		{},
		{},
		vk::ImageLayout::eUndefined
	);

	vk::Result r;

	std::tie(r, out->handle) = device->logical_device.createImage(image_create_info);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to create image object.");
		return nullptr;
	}

	// Query memory requirements
	vk::MemoryRequirements memory_reqs = device->logical_device.getImageMemoryRequirements(out->handle);

	// Get memory type index
	vk::PhysicalDeviceMemoryProperties memory_properties = device->physical_device_memory_properties;

	int32_t memory_type = -1;
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
	{
		if (memory_reqs.memoryTypeBits & (1 << i) &&
			(memory_properties.memoryTypes[i].propertyFlags & memory_flags) == memory_flags &&
			static_cast<uint32_t>(
				memory_properties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceCoherentAMD
			) == 0
		)
		{
			memory_type = i;
		}
	}

	if (memory_type == -1)
	{
		sl::log_error("Required memory type was not found.");
		return nullptr;
	}

	// Allocate memory
	vk::MemoryAllocateInfo memory_allocate_info(
		memory_reqs.size,
		memory_type
	);

	std::tie(r, out->memory) = device->logical_device.allocateMemory(memory_allocate_info);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to allocate memory for image.");
		return nullptr;
	}

	r = device->logical_device.bindImageMemory(out->handle, out->memory, 0);

	// Bind memory
	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to bind memory to image handle.");
		return nullptr;
	}

	if (create_view)
	{
		// Create view
		vk::ImageViewCreateInfo image_view_ci(
			{},
			out->handle,
			vk::ImageViewType::e2D,
			image_format,
			{},
			vk::ImageSubresourceRange(
				view_aspect_flags,
				0,
				1,
				0,
				1
			)
		);

		std::tie(r, out->image_view) = device->logical_device.createImageView(image_view_ci);

		if (r != vk::Result::eSuccess)
		{
			sl::log_error("Failed to create image view");
			return nullptr;
		}
	}

	return out;
}

Image::~Image()
{
	if (image_view)
	{
		device->logical_device.destroy(image_view);
	}

	if (memory)
	{
		device->logical_device.freeMemory(memory);
	}

	if (handle)
	{
		device->logical_device.destroy(handle);
	}
}

bool Image::transition_layout(const CommandBuffer* cb, vk::ImageLayout old_layout, vk::ImageLayout new_layout)
{
	vk::ImageMemoryBarrier barrier(
		{},
		{},
		old_layout,
		new_layout,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		handle,
		vk::ImageSubresourceRange(
			vk::ImageAspectFlagBits::eColor,
			0,
			1,
			0,
			1
		)
	);
	
	vk::PipelineStageFlags source_stage;
	vk::PipelineStageFlags dest_stage;

	if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		source_stage = vk::PipelineStageFlagBits::eTopOfPipe;

		dest_stage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (old_layout == vk::ImageLayout::eTransferDstOptimal &&
		new_layout == vk::ImageLayout::eShaderReadOnlyOptimal
	)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		source_stage = vk::PipelineStageFlagBits::eTransfer;

		dest_stage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else
	{
		sl::log_error("Unsupported layout transition.");
		return false;
	}

	cb->handle.pipelineBarrier(
			source_stage, dest_stage,
			{},
			0, nullptr,
			0, nullptr,
			1, &barrier
	);

	return true;
}

void Image::copy_from_buffer(const CommandBuffer* cb, vk::Buffer buffer)
{
	vk::BufferImageCopy buff_copy(
		0, 0, 0,
		vk::ImageSubresourceLayers(
			vk::ImageAspectFlagBits::eColor,
			0,
			0,
			1
		),
		{},
		{ size.w, size.h, 1 }
	);

	cb->handle.copyBufferToImage(buffer, handle, vk::ImageLayout::eTransferDstOptimal, 1, &buff_copy);
}

}
