#include "renderer/render_pass.hpp"

#include <stdexcept>
#include <vector>

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
	uint32_t stencil,
	uint8_t clear_flags,
	bool has_prev_pass,
	bool has_next_pass
) : device(device), render_area_start(render_area_start), render_area_size(render_area_size), clear_color(clear_color),
	depth(depth), stencil(stencil), clear_flags(clear_flags), has_prev_pass(has_prev_pass), has_next_pass(has_next_pass)
{
	// Main subpass
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	// Attachments TODO: make configurable.
	std::vector<VkAttachmentDescription> attachment_descriptions;
	attachment_descriptions.reserve(2);

	// Color attachment
	bool do_clear_color = clear_flags & RenderPassClearFlag::COLOR_BUFFER_FLAG;

	VkAttachmentDescription color_attachment = {};
	color_attachment.format = color_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = do_clear_color ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// If the attachment is coming from another pass, it should be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, otherwise
	// it should be undefined.
	color_attachment.initialLayout =
		has_prev_pass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;

	// If the attachment it going to another pass, it should be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, otherwise it
	// should be VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.
	color_attachment.finalLayout = 
		has_next_pass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	color_attachment.flags = 0;

	attachment_descriptions.push_back(color_attachment);

	VkAttachmentReference color_attachment_reference = {};
	color_attachment_reference.attachment = 0;  // Attachment description array index
	color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_reference;

	// Depth attachment, if there is one.
	bool do_clear_depth = clear_flags & RenderPassClearFlag::DEPTH_BUFFER_FLAG;
	
	VkAttachmentDescription depth_attachment = {};
	depth_attachment.format = depth_format;
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = do_clear_depth ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	if (do_clear_depth)
	{
		attachment_descriptions.push_back(depth_attachment);
	}

	// Depth attachment reference
	VkAttachmentReference depth_attachment_reference = {};
	depth_attachment_reference.attachment = 1;
	depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// TODO: other attachment types

	// Depth stencil data.
	subpass.pDepthStencilAttachment = do_clear_depth ? &depth_attachment_reference : nullptr;

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
	render_pass_create_info.attachmentCount = attachment_descriptions.size();
	render_pass_create_info.pAttachments = attachment_descriptions.data();
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

	bool do_clear_color = clear_flags & RenderPassClearFlag::COLOR_BUFFER_FLAG;
	
	if (do_clear_color)
	{
		clear_values[begin_info.clearValueCount].color.float32[0] = clear_color.r;
		clear_values[begin_info.clearValueCount].color.float32[1] = clear_color.g;
		clear_values[begin_info.clearValueCount].color.float32[2] = clear_color.b;
		clear_values[begin_info.clearValueCount].color.float32[3] = clear_color.a;

		begin_info.clearValueCount++;
	}

	bool do_clear_depth = clear_flags & RenderPassClearFlag::DEPTH_BUFFER_FLAG;

	if (do_clear_depth)
	{
		clear_values[begin_info.clearValueCount].color.float32[0] = clear_color.r;
		clear_values[begin_info.clearValueCount].color.float32[1] = clear_color.g;
		clear_values[begin_info.clearValueCount].color.float32[2] = clear_color.b;
		clear_values[begin_info.clearValueCount].color.float32[3] = clear_color.a;

		clear_values[begin_info.clearValueCount].depthStencil.depth = depth;

		bool do_clear_stencil = clear_flags & RenderPassClearFlag::STENCIL_BUFFER_FLAG;

		clear_values[begin_info.clearValueCount].depthStencil.stencil = do_clear_stencil ? stencil : 0;

		begin_info.clearValueCount++;
	}

	begin_info.pClearValues = begin_info.clearValueCount > 0 ? clear_values : nullptr;

	vkCmdBeginRenderPass(cb, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
	cb.set_state(CommandBufferState::IN_RENDER_PASS);
}

void RenderPass::end(CommandBuffer& cb)
{
	vkCmdEndRenderPass(cb);
	cb.set_state(CommandBufferState::RECORDING);
}

}
