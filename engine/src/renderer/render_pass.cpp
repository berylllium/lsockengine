#include "renderer/render_pass.hpp"

#include <stdexcept>
#include <vector>

#include <simple-logger.hpp>

namespace lise
{

std::unique_ptr<RenderPass> RenderPass::create(
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
)
{
	auto out = std::make_unique<RenderPass>();

	// Copy trivial data.
	out->device = device;
	out->render_area_start = render_area_start;
	out->render_area_size = render_area_size;
	out->clear_color = clear_color;
	out->depth = depth;
	out->stencil = stencil;
	out->clear_flags = clear_flags;
	out->has_prev_pass = has_prev_pass;
	out->has_next_pass = has_next_pass;

	// Main subpass
	vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics);

	// Attachments TODO: make configurable.
	std::vector<vk::AttachmentDescription> attachment_descriptions;
	attachment_descriptions.reserve(2);

	// Color attachment
	bool do_clear_color = clear_flags & RenderPassClearFlagBits::COLOR_BUFFER_FLAG;

	vk::AttachmentDescription color_attachment(
		{},
		color_format,
		vk::SampleCountFlagBits::e1,
		do_clear_color ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad,
		vk::AttachmentStoreOp::eStore,
		vk::AttachmentLoadOp::eDontCare,
		vk::AttachmentStoreOp::eDontCare,

		// If the attachment is coming from another pass, it should be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, otherwise
		// it should be undefined.
		has_prev_pass ? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::eUndefined,

		// If the attachment it going to another pass, it should be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, otherwise it
		// should be VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.
		has_next_pass ? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::ePresentSrcKHR
	);

	attachment_descriptions.push_back(color_attachment);

	vk::AttachmentReference color_attachment_reference(
		0,
		vk::ImageLayout::eColorAttachmentOptimal
	);

	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_reference;

	// Depth attachment, if there is one.
	bool do_clear_depth = clear_flags & RenderPassClearFlagBits::DEPTH_BUFFER_FLAG;
	
	vk::AttachmentDescription depth_attachment(
		{},
		depth_format,
		vk::SampleCountFlagBits::e1,
		do_clear_depth ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad,
		vk::AttachmentStoreOp::eDontCare,
		vk::AttachmentLoadOp::eDontCare,
		vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthStencilAttachmentOptimal
	);

	if (do_clear_depth)
	{
		attachment_descriptions.push_back(depth_attachment);
	}

	// Depth attachment reference
	vk::AttachmentReference depth_attachment_reference(
		1,
		vk::ImageLayout::eDepthStencilAttachmentOptimal
	);

	// TODO: other attachment types

	// Depth stencil data.
	subpass.pDepthStencilAttachment = do_clear_depth ? &depth_attachment_reference : nullptr;

	// Input from a shader
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = nullptr;

	// Attachments used for multisampling colour attachments
	subpass.pResolveAttachments = nullptr;

	// Attachments not used in this subpass, but must be preserved for the next.
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = nullptr;

	// Render pass dependencies. TODO: make this configurable.
	vk::SubpassDependency dependency(
		VK_SUBPASS_EXTERNAL,
		0,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		{},
		vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
		{}
	);

	// Render pass create.
	vk::RenderPassCreateInfo render_pass_create_info(
		{},
		attachment_descriptions.size(),
		attachment_descriptions.data(),
		1,
		&subpass,
		1,
		&dependency
	);

	vk::Result r;

	std::tie(r, out->handle) = out->device->logical_device.createRenderPass(render_pass_create_info);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to create render pass.");
		return nullptr;
	};

	return out;
}

RenderPass::~RenderPass()
{
	device->logical_device.destroy(handle);
}

void RenderPass::begin(CommandBuffer* cb, vk::Framebuffer frame_buffer)
{
	vk::RenderPassBeginInfo begin_info(
		handle,
		frame_buffer,
		{
			{ static_cast<int32_t>(render_area_start.x), static_cast<int32_t>(render_area_start.y) },
			{ render_area_size.w, render_area_size.h }
		}
	);

	vk::ClearValue clear_values[2] {};

	bool do_clear_color = clear_flags & RenderPassClearFlagBits::COLOR_BUFFER_FLAG;
	
	if (do_clear_color)
	{
		clear_values[begin_info.clearValueCount].color.float32[0] = clear_color.r;
		clear_values[begin_info.clearValueCount].color.float32[1] = clear_color.g;
		clear_values[begin_info.clearValueCount].color.float32[2] = clear_color.b;
		clear_values[begin_info.clearValueCount].color.float32[3] = clear_color.a;

		begin_info.clearValueCount++;
	}

	bool do_clear_depth = clear_flags & RenderPassClearFlagBits::DEPTH_BUFFER_FLAG;

	if (do_clear_depth)
	{
		clear_values[begin_info.clearValueCount].color.float32[0] = clear_color.r;
		clear_values[begin_info.clearValueCount].color.float32[1] = clear_color.g;
		clear_values[begin_info.clearValueCount].color.float32[2] = clear_color.b;
		clear_values[begin_info.clearValueCount].color.float32[3] = clear_color.a;

		clear_values[begin_info.clearValueCount].depthStencil.depth = depth;

		bool do_clear_stencil = clear_flags & RenderPassClearFlagBits::STENCIL_BUFFER_FLAG;

		clear_values[begin_info.clearValueCount].depthStencil.stencil = do_clear_stencil ? stencil : 0;

		begin_info.clearValueCount++;
	}

	begin_info.pClearValues = begin_info.clearValueCount > 0 ? clear_values : nullptr;

	cb->handle.beginRenderPass(begin_info, vk::SubpassContents::eInline);

	cb->set_state(CommandBufferState::IN_RENDER_PASS);
}

void RenderPass::end(CommandBuffer* cb)
{
	cb->handle.endRenderPass();

	cb->set_state(CommandBufferState::RECORDING);
}

}
