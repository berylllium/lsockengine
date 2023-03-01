#pragma once

#include <vulkan/vulkan.h>

#include "definitions.h"
#include "renderer/command_buffer.h"
#include "renderer/device.h"

typedef enum lise_render_pass_state
{
	LISE_RENDER_PASS_STATE_READY,
	LISE_RENDER_PASS_STATE_RECORDING,
	LISE_RENDER_PASS_STATE_IN_RENDER_PASS,
	LISE_RENDER_PASS_STATE_RECORDING_ENDED,
	LISE_RENDER_PASS_STATE_SUBMITTED,
	LISE_RENDER_PASS_STATE_NOT_ALLOCATED
} lise_render_pass_state;

typedef struct lise_render_pass
{
	VkRenderPass handle;

	float x, y, w, h; // Render area
	float r, g, b, a; // Clear color

	float depth;
	uint32_t stencil;

	lise_render_pass_state state;
} lise_render_pass;

bool lise_render_pass_create(
	lise_device* device,
	VkFormat color_format,
	VkFormat depth_format,
	float x, float y, float w, float h,
	float r, float g, float b, float a,
	float depth,
	float stencil,
	lise_render_pass* out_render_pass
);

bool lise_render_pass_recreate(
	lise_device* device,
	VkFormat color_format,
	VkFormat depth_format,
	float x, float y, float w, float h,
	float r, float g, float b, float a,
	float depth,
	float stencil,
	lise_render_pass* out_render_pass
);

void lise_render_pass_destroy(VkDevice device, lise_render_pass* render_pass);

void lise_render_pass_begin(
	lise_command_buffer* command_buffer,
	lise_render_pass* render_pass,
	VkFramebuffer frame_buffer
);

void lise_render_pass_end(lise_command_buffer* command_buffer, lise_render_pass* render_pass);
