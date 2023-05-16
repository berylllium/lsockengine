#include "renderer/vulkan_image.hpp"

#include <stdexcept>

#include "core/logger.hpp"

namespace lise
{

VulkanImage::VulkanImage(
	const Device& device,
	VkImageType image_type,
	vector2ui size,
	VkFormat image_format,
	VkImageTiling image_tiling,
	VkImageUsageFlags use_flags,
	VkMemoryPropertyFlags memory_flags,
	bool create_view,
	VkImageAspectFlags view_aspect_flags
) : size(size), image_format(image_format), device(device)
{
	VkImageCreateInfo image_create_info = {};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = image_type;
	image_create_info.extent.width = size.w;
	image_create_info.extent.height = size.h;
	image_create_info.extent.depth = 1; // TODO: Support configurable depth.
	image_create_info.mipLevels = 4;
	image_create_info.arrayLayers = 1;
	image_create_info.format = image_format;
	image_create_info.tiling = image_tiling;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_create_info.usage = use_flags;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(device, &image_create_info, NULL, &image_handle) != VK_SUCCESS)
	{
		LERROR("Failed to create image object.");
		throw std::exception();
	}

	// Query memory requirements
	VkMemoryRequirements memory_reqs;
	vkGetImageMemoryRequirements(device, image_handle, &memory_reqs);

	// Get memory type index
	VkPhysicalDeviceMemoryProperties memory_properties = device.get_memory_properties();

	int32_t memory_type = -1;
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
	{
		if (memory_reqs.memoryTypeBits & (1 << i) &&
			(memory_properties.memoryTypes[i].propertyFlags & memory_flags) == memory_flags &&
			(memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) == 0
		)
		{
			memory_type = i;
		}
	}

	if (memory_type == -1)
	{
		LERROR("Required memory type was not found.");
		throw std::exception();
	}

	// Allocate memory
	VkMemoryAllocateInfo memory_allocate_info = {};
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize = memory_reqs.size;
	memory_allocate_info.memoryTypeIndex = memory_type;

	if (vkAllocateMemory(device, &memory_allocate_info, NULL, &memory) != VK_SUCCESS)
	{
		LERROR("Failed to allocate memory for image.");
		throw std::exception();
	}

	// Bind memory
	if (vkBindImageMemory(device, image_handle, memory, 0) != VK_SUCCESS)
	{
		LERROR("Failed to bind memory to image handle.");
		throw std::exception();
	}

	if (create_view)
	{
		// Create view
		VkImageViewCreateInfo image_view_ci = {};
		image_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_ci.image = image_handle;
		image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D; // TODO: Support configurable viewType
		image_view_ci.format = image_format;
		image_view_ci.subresourceRange.aspectMask = view_aspect_flags;

		// TODO: Make this configurable
		image_view_ci.subresourceRange.baseMipLevel = 0;
 		image_view_ci.subresourceRange.levelCount = 1;
		image_view_ci.subresourceRange.baseArrayLayer = 0;
 		image_view_ci.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &image_view_ci, NULL, &image_view) != VK_SUCCESS)
		{
			LERROR("Failed to create image view");
			throw std::exception();
		}
	}
}

VulkanImage::VulkanImage(VulkanImage&& other) : device(other.device), size(other.size)
{
	image_handle = other.image_handle;
	other.image_handle = nullptr;

	memory = other.memory;
	other.memory = nullptr;

	image_view = other.image_view;
	other.image_view = nullptr;
}

VulkanImage::~VulkanImage()
{
	if (image_view)
	{
		vkDestroyImageView(device, image_view, NULL);
	}

	if (memory)
	{
		vkFreeMemory(device, memory, NULL);
	}

	if (image_handle)
	{
		vkDestroyImage(device, image_handle, NULL);
	}
}

void VulkanImage::transition_layout(
	const CommandBuffer& cb,
	VkImageLayout old_layout,
	VkImageLayout new_layout
)
{
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image_handle;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags source_stage;
	VkPipelineStageFlags dest_stage;

	if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

		dest_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

		dest_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		std::runtime_error("Unsupported layout transition.");
	}

	vkCmdPipelineBarrier(
		cb,
		source_stage, dest_stage,
		0,
		0, 0,
		0, 0,
		1, &barrier
	);
}

void VulkanImage::copy_from_buffer(const CommandBuffer& cb, VkBuffer buffer)
{
	VkBufferImageCopy buff_copy = {};
	buff_copy.bufferOffset = 0;
	buff_copy.bufferRowLength = 0;
	buff_copy.bufferImageHeight = 0;

	buff_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	buff_copy.imageSubresource.mipLevel = 0;
	buff_copy.imageSubresource.baseArrayLayer = 0;
	buff_copy.imageSubresource.layerCount = 1;

	buff_copy.imageExtent.width = size.w;
	buff_copy.imageExtent.height = size.h;
	buff_copy.imageExtent.depth = 1;

	vkCmdCopyBufferToImage(
		cb,
		buffer,
		image_handle,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&buff_copy
	);
}

VkImageView VulkanImage::get_image_view() const
{
	return image_view;
}

}
