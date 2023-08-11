#pragma once

#include <string>

#include "definitions.hpp"
#include "renderer/device.hpp"
#include "renderer/vulkan_image.hpp"

namespace lise
{

struct Texture
{
	std::string path;

	vector2ui size;
	uint8_t channel_count;

	std::unique_ptr<Image> image;
	vk::Sampler sampler;

	const Device* device;

	Texture() = default;

	Texture(const Texture&) = delete; // Prevent copies.

	~Texture();

	Texture& operator = (const Texture&) = delete; // Prevent copies.

	LAPI static std::unique_ptr<Texture> create(
		const Device* device,
		const std::string& path,
		vector2ui size,
		uint32_t channel_count,
		const uint8_t* data,
		bool has_transparency
	);
};

}
