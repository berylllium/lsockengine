#pragma once

#include "definitions.hpp"
#include "math/vector2.hpp"
#include "renderer/device.hpp"
#include "renderer/command_buffer.hpp"

namespace lise
{

struct Image
{
	vector2ui size;

	vk::Image handle;

	vk::DeviceMemory memory;
	vk::ImageView image_view;

	vk::Format image_format;

	const Device* device;

	Image() = default;

	Image(Image&) = delete; // Prevent copies.

	~Image();

	Image& operator = (const Image&) = delete; // Prevent copies.
	
	LAPI static std::unique_ptr<Image> create(
		const Device* device,
		vk::ImageType image_type,
		vector2ui size,
		vk::Format image_format,
		vk::ImageTiling image_tiling,
		vk::ImageUsageFlags use_flags,
		vk::MemoryPropertyFlags memory_flags,
		bool create_view,
		vk::ImageAspectFlags view_aspect_flags
	);

	bool transition_layout(const CommandBuffer* command_buffer, vk::ImageLayout old_layout, vk::ImageLayout new_layout);

	void copy_from_buffer(const CommandBuffer* command_buffer, vk::Buffer buffer);
};

}
