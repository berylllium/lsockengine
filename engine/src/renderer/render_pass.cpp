#include "renderer/render_pass.hpp"

#include <stdexcept>

namespace lise
{

RenderPass::RenderPass(
	const Device& device,
	VkFormat color_format,
	VkFormat depth_format,
	vector2ui render_area_start,
	vector2ui render_area_size,
	vector4f clear_color,
	float depth,
	uint32_t stencil
) : device(device), render_area_start(render_area_start), render_area_size(render_area_size), clear_color(clear_color),
	depth(depth), stencil(stencil)
{
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

	if (vkCreateRenderPass(device, &render_pass_create_info, NULL, &handle) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass.");
	};
}

RenderPass::RenderPass(RenderPass&& other) : device(other.device), render_area_start(other.render_area_start),
	render_area_size(other.render_area_size), clear_color(other.clear_color), depth(other.depth),
	stencil(other.stencil), state(other.state)
{
	this->~RenderPass();

	handle = other.handle;
	other.handle = nullptr;
}

RenderPass::~RenderPass()
{
	vkDestroyRenderPass(device, handle, NULL);
	handle = nullptr;
}

RenderPass::operator VkRenderPass() const
{
	return handle;
}

void RenderPass::begin(CommandBuffer& cb, VkFramebuffer frame_buffer)
{
	VkRenderPassBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	begin_info.renderPass = handle;
	begin_info.framebuffer = frame_buffer;
	begin_info.renderArea.offset.x = render_area_start.x;
	begin_info.renderArea.offset.y = render_area_start.y;
	begin_info.renderArea.extent.width = render_area_size.w;
	begin_info.renderArea.extent.height = render_area_size.h;

	VkClearValue clear_values[2] {};
	clear_values[0].color.float32[0] = clear_color.r;
	clear_values[0].color.float32[1] = clear_color.g;
	clear_values[0].color.float32[2] = clear_color.b;
	clear_values[0].color.float32[3] = clear_color.a;
	clear_values[1].depthStencil.depth = depth;
	clear_values[1].depthStencil.stencil = stencil;

	begin_info.clearValueCount = 2;
	begin_info.pClearValues = clear_values;

	vkCmdBeginRenderPass(cb, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
	cb.set_state(CommandBufferState::IN_RENDER_PASS);
}

void RenderPass::end(CommandBuffer& cb)
{
	vkCmdEndRenderPass(cb);
	cb.set_state(CommandBufferState::RECORDING);
}

}
