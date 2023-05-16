#pragma once

#include <vulkan/vulkan.h>

#include "definitions.hpp"
#include "math/vector2.hpp"
#include "renderer/render_pass.hpp"

namespace lise
{

class Framebuffer
{
public:
	Framebuffer(
		VkDevice device,
		const RenderPass& render_pass,
		vector2ui size,
		uint32_t attachment_count,
		VkImageView* attachments
	);

	Framebuffer(Framebuffer&& other);

	Framebuffer(const Framebuffer&) = delete; // Prevent copies.

	~Framebuffer();

private:
	VkFramebuffer handle;

	uint32_t attachment_count;
	VkImageView* attachments;

	const RenderPass& render_pass;

	const VkDevice& device;
};

}
