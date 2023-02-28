#include "renderer/render_pass.h"

#include <string.h>

#include "core/logger.h"

bool lise_render_pass_create(
	lise_device* device,
	VkFormat color_format,
	VkFormat depth_format,
	float x, float y, float w, float h,
	float r, float g, float b, float a,
	float depth,
	float stencil,
	lise_render_pass* out_render_pass
)
{
	out_render_pass->x = x;
	out_render_pass->y = y;
	out_render_pass->w = w;
	out_render_pass->h = h;

	out_render_pass->r = r;
	out_render_pass->g = g;
	out_render_pass->b = b;
	out_render_pass->a = a;

	out_render_pass->depth = depth;
	out_render_pass->stencil = stencil;

	// Main subpass
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	// Attachments TODO: make configurable.
	uint32_t attachment_description_count = 2;
	VkAttachmentDescription attachment_descriptions[2];

	// Color attachment
	VkAttachmentDescription color_attachment = {};
	color_attachment.format = color_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;	  // Do not expect any particular layout before render pass starts.
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  // Transitioned to after the render pass
	color_attachment.flags = 0;

	attachment_descriptions[0] = color_attachment;

	VkAttachmentReference color_attachment_reference = {};
	color_attachment_reference.attachment = 0;  // Attachment description array index
	color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_reference;

	// Depth attachment, if there is one
	VkAttachmentDescription depth_attachment = {};
	depth_attachment.format = depth_format;
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	attachment_descriptions[1] = depth_attachment;

	// Depth attachment reference
	VkAttachmentReference depth_attachment_reference = {};
	depth_attachment_reference.attachment = 1;
	depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// TODO: other attachment types

	// Depth stencil data.
	subpass.pDepthStencilAttachment = &depth_attachment_reference;

	// Input from a shader
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = 0;

	// Attachments used for multisampling colour attachments
	subpass.pResolveAttachments = 0;

	// Attachments not used in this subpass, but must be preserved for the next.
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = 0;

	// Render pass dependencies. TODO: make this configurable.
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dependencyFlags = 0;

	// Render pass create.
	VkRenderPassCreateInfo render_pass_create_info = {};
	render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_create_info.attachmentCount = attachment_description_count;
	render_pass_create_info.pAttachments = attachment_descriptions;
	render_pass_create_info.subpassCount = 1;
	render_pass_create_info.pSubpasses = &subpass;
	render_pass_create_info.dependencyCount = 1;
	render_pass_create_info.pDependencies = &dependency;
	render_pass_create_info.pNext = 0;
	render_pass_create_info.flags = 0;

	if (vkCreateRenderPass(device->logical_device, &render_pass_create_info, NULL, &out_render_pass->handle) 
		!= VK_SUCCESS
	)
	{
		LERROR("Failed to create render pass.");
		
		return false;
	};

	return true;
}

void lise_render_pass_destroy(VkDevice device, lise_render_pass* render_pass)
{
	if (render_pass && render_pass->handle)
	{
		vkDestroyRenderPass(device, render_pass->handle, NULL);
		render_pass->handle = NULL;
	}
}

void lise_render_pass_begin(
	lise_command_buffer* command_buffer,
	lise_render_pass* render_pass,
	VkFramebuffer frame_buffer
)
{
	VkRenderPassBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	begin_info.renderPass = render_pass->handle;
	begin_info.framebuffer = frame_buffer;
	begin_info.renderArea.offset.x = render_pass->x;
	begin_info.renderArea.offset.y = render_pass->y;
	begin_info.renderArea.extent.width = render_pass->w;
	begin_info.renderArea.extent.height = render_pass->h;

	VkClearValue clear_values[2];
	memset(clear_values, 0, sizeof(VkClearValue) * 2);
	clear_values[0].color.float32[0] = render_pass->r;
	clear_values[0].color.float32[1] = render_pass->g;
	clear_values[0].color.float32[2] = render_pass->b;
	clear_values[0].color.float32[3] = render_pass->a;
	clear_values[1].depthStencil.depth = render_pass->depth;
	clear_values[1].depthStencil.stencil = render_pass->stencil;

	begin_info.clearValueCount = 2;
	begin_info.pClearValues = clear_values;

	vkCmdBeginRenderPass(command_buffer->handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
	command_buffer->state = LISE_COMMAND_BUFFER_STATE_IN_RENDER_PASS;
}

void lise_render_pass_end(lise_command_buffer* command_buffer, lise_render_pass* render_pass)
{
	vkCmdEndRenderPass(command_buffer->handle);
	command_buffer->state = LISE_COMMAND_BUFFER_STATE_RECORDING;
}
