#pragma once

#include <string>

#include "definitions.hpp"
#include "renderer/device.hpp"
#include "renderer/vulkan_image.hpp"

namespace lise
{

class Texture
{
public:
	const std::string path;

	const vector2ui size;
	const uint8_t channel_count;

	Texture(
		const Device& device,
		const std::string& path,
		vector2ui size,
		uint32_t channel_count,
		const uint8_t* data,
		bool has_transparency
	);

	Texture(Texture&& other);

	Texture(const Texture&) = delete; // Prevent copies.

	~Texture();

	operator const VulkanImage&() const;

	Texture& operator = (const Texture&) = delete; // Prevent copies.

	VkSampler get_sampler() const;

private:
	VulkanImage* image;
	VkSampler sampler;

	const Device& device;
};

}
