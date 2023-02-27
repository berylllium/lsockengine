#include "renderer/vulkan_image.h"

#include "core/logger.h"

bool lise_vulkan_image_create(
	lise_device* device,
	VkImageType image_type,
	uint32_t width,
	uint32_t height,
	VkFormat image_format,
	VkImageTiling image_tiling,
	VkImageUsageFlags use_flags,
	VkMemoryPropertyFlags memory_flags,
	bool create_view,
	VkImageAspectFlags view_aspect_flags,
	lise_vulkan_image* out_image
)
{
	// Copy parameters
	out_image->width = width;
	out_image->height = height;

	VkImageCreateInfo image_create_info = {};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = image_type;
	image_create_info.extent.width = width;
	image_create_info.extent.height = height;
	image_create_info.extent.depth = 1; // TODO: Support configurable depth.
	image_create_info.mipLevels = 4;
	image_create_info.arrayLayers = 1;
	image_create_info.format = image_format;
	image_create_info.tiling = image_tiling;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_create_info.usage = use_flags;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(device->logical_device, &image_create_info, NULL, &out_image->image_handle) != VK_SUCCESS)
	{
		LERROR("Failed to create image object.");
		return false;
	}

	// Query memory requirements
	VkMemoryRequirements memory_reqs;
	vkGetImageMemoryRequirements(device->logical_device, out_image->image_handle, &memory_reqs);

	// Get memory type index
	int32_t memory_type = -1;
	for (uint32_t i = 0; i < device->physical_device_memory_properties.memoryTypeCount; i++)
	{
		if (memory_reqs.memoryTypeBits & (1 << i) &&
			(device->physical_device_memory_properties.memoryTypes[i].propertyFlags & memory_flags) == memory_flags &&
			(device->physical_device_memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) == 0)
		{
			memory_type = i;
		}
	}

	if (memory_type == -1)
	{
		LERROR("Required memory type was not found.");
		return false;
	}

	// Allocate memory
	VkMemoryAllocateInfo memory_allocate_info = {};
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize = memory_reqs.size;
	memory_allocate_info.memoryTypeIndex = memory_type;

	if (vkAllocateMemory(device->logical_device, &memory_allocate_info, NULL, &out_image->memory) != VK_SUCCESS)
	{
		LERROR("Failed to allocate memory for image.");
		return false;
	}

	// Bind memory
	if (vkBindImageMemory(device->logical_device, out_image->image_handle, out_image->memory, 0) != VK_SUCCESS)
	{
		LERROR("Failed to bind memory to image handle.");
		return false;
	}

	if (create_view)
	{
		// Create view
		VkImageViewCreateInfo image_view_ci = {};
		image_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_ci.image = out_image->image_handle;
		image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D; // TODO: Support configurable viewType
		image_view_ci.format = image_format;
		image_view_ci.subresourceRange.aspectMask = view_aspect_flags;

		// TODO: Make this configurable
		image_view_ci.subresourceRange.baseMipLevel = 0;
 		image_view_ci.subresourceRange.levelCount = 1;
		image_view_ci.subresourceRange.baseArrayLayer = 0;
 		image_view_ci.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device->logical_device, &image_view_ci, NULL, &out_image->image_view) != VK_SUCCESS)
		{
			LERROR("Failed to create image view");
			return false;
		}
	}

	return true;
}

void lise_vulkan_image_destroy(VkDevice device, lise_vulkan_image* image)
{
	if (image->image_view)
	{
		vkDestroyImageView(device, image->image_view, NULL);
		image->image_view = NULL;
	}

	if (image->memory)
	{
		vkFreeMemory(device, image->memory, NULL);
		image->memory = NULL;
	}

	if (image->image_handle)
	{
		vkDestroyImage(device, image->image_handle, NULL);
		image->image_handle = NULL;
	}
}
