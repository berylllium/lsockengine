#include "renderer/framebuffer.hpp"

#include <cstdlib>
#include <algorithm>

#include <simple-logger.hpp>

namespace lise
{

Framebuffer::Framebuffer(
	const Device& device,
	const RenderPass& render_pass,
	vector2ui size,
	uint32_t attachment_count,
	VkImageView* attachments
) : attachment_count(attachment_count), render_pass(render_pass), device(device)
{
	// Make copies of the attachments
	this->attachments = new VkImageView[attachment_count];
	std::copy(attachments, attachments + attachment_count, this->attachments);

	VkFramebufferCreateInfo framebuffer_ci = {};
	framebuffer_ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_ci.renderPass = render_pass;
	framebuffer_ci.attachmentCount = attachment_count;
	framebuffer_ci.pAttachments = attachments;
	framebuffer_ci.width = size.w;
	framebuffer_ci.height = size.h;
	framebuffer_ci.layers = 1;

	if (vkCreateFramebuffer(device, &framebuffer_ci, NULL, &handle) != VK_SUCCESS)
	{
		sl::log_error("Failed to create framebuffer.");
		throw std::exception();
	}
}

Framebuffer::Framebuffer(Framebuffer&& other) : device(other.device), render_pass(other.render_pass)
{
	handle = other.handle;
	other.handle = nullptr;

	attachment_count = other.attachment_count;
	other.attachment_count = 0;

	attachments = other.attachments;
	other.attachments = nullptr;
}

Framebuffer::~Framebuffer()
{
	if (handle)
	{
		vkDestroyFramebuffer(device, handle, NULL);
	}

	delete attachments;
}

Framebuffer::operator VkFramebuffer() const
{
	return handle;
}

}
