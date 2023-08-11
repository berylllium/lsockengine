#include "renderer/pipeline.hpp"

#include <simple-logger.hpp>

namespace lise
{

std::unique_ptr<Pipeline> Pipeline::create(
	const Device* device,
	const RenderPass* render_pass,
	uint32_t vertex_input_stride,
	const std::vector<vk::VertexInputAttributeDescription>& attributes,
	const std::vector<vk::DescriptorSetLayout>& descriptor_set_layouts,
	const std::vector<vk::PipelineShaderStageCreateInfo>& shader_stages,
	const std::vector<vk::PushConstantRange>& push_constant_ranges,
	vk::Viewport viewport,
	vk::Rect2D scissor,
	bool is_wireframe,
	bool depth_test_enabled
)
{
	auto out = std::make_unique<Pipeline>();

	// Copy trivial data.
	out->device = device;

	// Viewport state
	vk::PipelineViewportStateCreateInfo viewport_state(
		{},
		1,
		&viewport,
		1,
		&scissor
	);

	// Rasterizer
	vk::PipelineRasterizationStateCreateInfo rasterizer_create_info(
		{},
		vk::False,
		vk::False,
		is_wireframe ? vk::PolygonMode::eLine : vk::PolygonMode::eFill,
		vk::CullModeFlagBits::eBack,
		vk::FrontFace::eCounterClockwise,
		vk::False,
		0.0f,
		0.0f,
		0.0f,
		1.0f
	);

	// Multisampling.
	vk::PipelineMultisampleStateCreateInfo multisampling_create_info(
		{},
		vk::SampleCountFlagBits::e1,
		vk::False,
		1.0f,
		nullptr,
		vk::False,
		vk::False
	);

	// Depth and stencil testing.
	vk::PipelineDepthStencilStateCreateInfo depth_stencil(
		{},
		vk::True,
		vk::True,
		vk::CompareOp::eLess,
		vk::False,
		vk::False
	);

	vk::PipelineColorBlendAttachmentState color_blend_attachment_state(
		vk::True,
		vk::BlendFactor::eSrcAlpha,
		vk::BlendFactor::eOneMinusSrcAlpha,
		vk::BlendOp::eAdd,
		vk::BlendFactor::eSrcAlpha,
		vk::BlendFactor::eOneMinusSrcAlpha,
		vk::BlendOp::eAdd,
		vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
			vk::ColorComponentFlagBits::eA
	);

	vk::PipelineColorBlendStateCreateInfo color_blend_state_create_info(
		{},
		vk::False,
		vk::LogicOp::eCopy,
		1,
		&color_blend_attachment_state
	);

	// Dynamic state
	const uint32_t dynamic_state_count = 3;
	vk::DynamicState dynamic_states[3] = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor,
		vk::DynamicState::eLineWidth
	};

	vk::PipelineDynamicStateCreateInfo dynamic_state_create_info(
		{},
		dynamic_state_count,
		dynamic_states
	);

	// Vertex input
	vk::VertexInputBindingDescription binding_description(
		0,
		vertex_input_stride,
		vk::VertexInputRate::eVertex
	);

	// Attributes
	vk::PipelineVertexInputStateCreateInfo vertex_input_info(
		{},
		1,
		&binding_description,
		attributes.size(),
		attributes.data()
	);

	// Input assembly
	vk::PipelineInputAssemblyStateCreateInfo input_assembly(
		{},
		vk::PrimitiveTopology::eTriangleList,
		vk::False
	);

	// Pipeline layout
	vk::PipelineLayoutCreateInfo pipeline_layout_create_info(
		{},
		descriptor_set_layouts,
		push_constant_ranges
	);

	// Create the pipeline layout.
	vk::Result r;

	std::tie(r, out->pipeline_layout) = out->device->logical_device.createPipelineLayout(pipeline_layout_create_info);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to create pipline layout.");
		return nullptr;
	}

	// Pipeline create
	vk::GraphicsPipelineCreateInfo pipeline_create_info(
		{},
		shader_stages,
		&vertex_input_info,
		&input_assembly,
		nullptr,
		&viewport_state,
		&rasterizer_create_info,
		&multisampling_create_info,
		depth_test_enabled ? &depth_stencil : nullptr,
		&color_blend_state_create_info,
		&dynamic_state_create_info,
		
		out->pipeline_layout,

		render_pass->handle,
		0,
		nullptr,
		-1
	);

	std::tie(r, out->handle) = out->device->logical_device.createGraphicsPipeline({}, pipeline_create_info);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to create the graphics pipeline.");
		return nullptr;
	}

	return out;
}

Pipeline::~Pipeline()
{
	if (handle)
	{
		device->logical_device.destroy(handle);
	}

	if (pipeline_layout)
	{
		device->logical_device.destroy(pipeline_layout);
	}
}

void Pipeline::bind(const CommandBuffer* command_buffer, vk::PipelineBindPoint bind_point)
{
	command_buffer->handle.bindPipeline(bind_point, handle);
}

}
