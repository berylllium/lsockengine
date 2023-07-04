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

enum RenderPassClearFlag
{
	NONE_FLAG = 0x0,
	COLOR_BUFFER_FLAG = 0x1,
	DEPTH_BUFFER_FLAG = 0x2,
	STENCIL_BUFFER_FLAG = 0x4
};

class RenderPass
{
public:
	RenderPass(
		const Device& device,
		VkFormat color_format,
		VkFormat depth_format,
		vector2ui render_area_start,
		vector2ui render_area_size,
		vector4f clear_color,
		float depth,
		uint32_t stencil,
		uint8_t clear_flags,
		bool has_prev_pass,
		bool has_next_pass
	);

	RenderPass(RenderPass&& other);

	RenderPass(RenderPass&) = delete; // Prevent copies.

	~RenderPass();

	operator VkRenderPass() const;

	void begin(CommandBuffer& cb, VkFramebuffer frame_buffer);

	void end(CommandBuffer& cb);

private:
	VkRenderPass handle;

	vector2ui render_area_start;
	vector2ui render_area_size;

	vector4f clear_color;

	float depth;
	uint32_t stencil;

	uint8_t clear_flags;
	bool has_prev_pass;
	bool has_next_pass;

	render_pass_state state;

	const Device& device;
};

}
