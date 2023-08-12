#pragma once

#include "definitions.hpp"
#include "renderer/command_buffer.hpp"
#include "renderer/device.hpp"
#include "math/vector2.hpp"
#include "math/vector4.hpp"

namespace lise
{

enum class RenderPassState
{
	READY,
	RECORDING,
	IN_RENDER_PASS,
	RECORDING_ENDED,
	SUBMITTED,
	NOT_ALLOCATED
};

enum RenderPassClearFlagBits
{
	NONE_FLAG = 0x0,
	COLOR_BUFFER_FLAG = 0x1,
	DEPTH_BUFFER_FLAG = 0x2,
	STENCIL_BUFFER_FLAG = 0x4
};

struct RenderPass
{
	vk::RenderPass handle;

	vector2ui render_area_start;
	vector2ui render_area_size;

	vector4f clear_color;

	float depth;
	uint32_t stencil;

	uint8_t clear_flags;
	bool has_prev_pass;
	bool has_next_pass;

	RenderPassState state;

	const Device* device;

	RenderPass() = default;

	RenderPass(RenderPass&) = delete; // Prevent copies.

	~RenderPass();

	LAPI static std::unique_ptr<RenderPass> create(
		const Device* device,
		vk::Format color_format,
		vk::Format depth_format,
		vector2ui render_area_start,
		vector2ui render_area_size,
		vector4f clear_color,
		float depth,
		uint32_t stencil,
		uint8_t clear_flags,
		bool has_prev_pass,
		bool has_next_pass
	);

	void begin(CommandBuffer* cb, vk::Framebuffer frame_buffer);

	void end(CommandBuffer* cb);
};

}
