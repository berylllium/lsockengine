#pragma once

#include <vulkan/vulkan.h>

#include "definitions.hpp"
#include "renderer/command_buffer.hpp"
#include "renderer/device.hpp"
#include "math/vector2.hpp"
#include "math/vector4.hpp"

namespace lise
{

enum class render_pass_state
{
	READY,
	RECORDING,
	IN_RENDER_PASS,
	RECORDING_ENDED,
	SUBMITTED,
	NOT_ALLOCATED
};

class RenderPass
{
public:
	RenderPass(
		const Device& device,
		VkFormat color_format,
		VkFormat depth_format,
		vector2i render_area_start,
		vector2i render_area_size,
		vector4f clear_color,
		float depth,
		uint32_t stencil
	);

	RenderPass(RenderPass&& other);

	RenderPass(RenderPass&) = delete; // Prevent copies.

	~RenderPass();

	operator VkRenderPass() const;

	void begin(CommandBuffer& cb, VkFramebuffer frame_buffer);

	void end(CommandBuffer& cb);

private:
	VkRenderPass handle;

	vector2i render_area_start;
	vector2i render_area_size;

	vector4f clear_color;

	float depth;
	uint32_t stencil;

	render_pass_state state;

	const Device& device;
};

}
