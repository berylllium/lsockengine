#include "renderer/framebuffer.h"

#include <stdlib.h>
#include <string.h>

#include "core/logger.h"

bool lise_framebuffer_create(
	VkDevice device,
	lise_render_pass* render_pass,
	uint32_t width,
	uint32_t height,
	uint32_t attachment_count,
	VkImageView* attachments,
	lise_framebuffer* out_framebuffer
)
{
	// Make copies of the attachments
	out_framebuffer->attachment_count = attachment_count;

	out_framebuffer->attachments = malloc(sizeof(VkImageView) * attachment_count);
	memcpy(out_framebuffer->attachments, attachments, sizeof(VkImageView) * attachment_count);

	// Copy over the render pass pointer
	out_framebuffer->render_pass = render_pass;

	VkFramebufferCreateInfo framebuffer_ci = {};
	framebuffer_ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_ci.renderPass = render_pass->handle;
	framebuffer_ci.attachmentCount = attachment_count;
	framebuffer_ci.pAttachments = out_framebuffer->attachments;
	framebuffer_ci.width = width;
	framebuffer_ci.height = height;
	framebuffer_ci.layers = 1;

	if (vkCreateFramebuffer(device, &framebuffer_ci, NULL, &out_framebuffer->handle) != VK_SUCCESS)
	{
		LERROR("Failed to create framebuffer.");
		return false;
	}

	return true;
}

void lise_framebuffer_destroy(VkDevice device, lise_framebuffer* framebuffer)
{
	vkDestroyFramebuffer(device, framebuffer->handle, NULL);

	if (framebuffer->attachments)
	{
		free(framebuffer->attachments);
	}

	memset(framebuffer, 0, sizeof(framebuffer));
}
