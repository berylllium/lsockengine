#pragma once

#include <vulkan/vulkan.h>

#include "definitions.h"
#include "renderer/render_pass.h"

typedef struct lise_framebuffer
{
	VkFramebuffer handle;

	uint32_t attachment_count;
	VkImageView* attachments;

	lise_render_pass* render_pass;
} lise_framebuffer;

bool lise_framebuffer_create(
	VkDevice device,
	lise_render_pass* render_pass,
	uint32_t width,
	uint32_t height,
	uint32_t attachment_count,
	VkImageView* attachments,
	lise_framebuffer* out_framebuffer
);

void lise_framebuffer_destroy(VkDevice device, lise_framebuffer* framebuffer);
