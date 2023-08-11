#include "renderer/resource/texture.hpp"

#include <simple-logger.hpp>

#include "renderer/vulkan_buffer.hpp"

namespace lise
{

std::unique_ptr<Texture> Texture::create(
	const Device* device,
	const std::string& path,
	vector2ui size,
	uint32_t channel_count,
	const uint8_t* data,
	bool has_transparency
)
{
	auto out = std::make_unique<Texture>();

	// Copy trivial data.
	out->device = device;
	out->path = path;
	out->size = size;
	out->channel_count = channel_count;

	uint64_t byte_size = size.w * size.h * channel_count;

	// Assume format.
	vk::Format image_format = vk::Format::eR8G8B8A8Unorm;
	
	// Create a staging buffer and load texture data into it.
	auto staging_buffer = VulkanBuffer::create(
		device,
		byte_size,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		true
	);

	staging_buffer->load_data(0, byte_size, {}, data);

	// Create the image. A lot of assumptions are made here to work with the PNG format.
	out->image = Image::create(
		device,
		vk::ImageType::e2D,
		size,
		image_format,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled |
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		true,
		vk::ImageAspectFlagBits::eColor
	);

	// Create and begin a temporary buffer.
	auto temp_buffer = CommandBuffer::create(device, device->graphics_command_pool, true);

	if (!temp_buffer)
	{
		sl::log_error("Failed failed to create temporary command buffer.");
		return nullptr;
	}

	temp_buffer->begin(true, false, false);

	// Transition the image layout from undefined to optimal for receiving data.
	out->image->transition_layout(temp_buffer.get(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

	// Copy the data from the staging buffer to the image buffer.
	out->image->copy_from_buffer(temp_buffer.get(), staging_buffer->handle);

	// Transition the image layout from optimal for receiving data to a shader read only optimal layout.
	out->image->transition_layout(
		temp_buffer.get(),
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal
	);

	// End and submit the temporary command buffer.
	temp_buffer->end_and_submit_single_use(device->graphics_queue);

	// Create a sampler for the texture.
	vk::SamplerCreateInfo sampler_ci(
		{},
		vk::Filter::eLinear,
		vk::Filter::eLinear,
		vk::SamplerMipmapMode::eLinear,
		vk::SamplerAddressMode::eRepeat,
		vk::SamplerAddressMode::eRepeat,
		vk::SamplerAddressMode::eRepeat,
		0.0f,
		vk::False,
		16.0f,
		vk::False,
		vk::CompareOp::eAlways,
		0.0f,
		0.0f,
		vk::BorderColor::eIntOpaqueBlack,
		vk::False
	);

	vk::Result r;

	std::tie(r, out->sampler) = device->logical_device.createSampler(sampler_ci);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to create a sampler for the following texture: `%s`.", path);
		return nullptr;
	}

	return out;
}

Texture::~Texture()
{
	if (sampler)
	{
		device->logical_device.destroy(sampler);
	}
}

}
