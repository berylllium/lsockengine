#pragma once

#include <vulkan/vulkan.h>

#include "definitions.hpp"
#include "renderer/command_buffer.hpp"
#include "renderer/device.hpp"
#include "renderer/render_pass.hpp"

namespace lise
{

class Pipeline
{
public:
	Pipeline(
		const Device& device,
		const RenderPass& render_pass,
		uint32_t attribute_count,
		VkVertexInputAttributeDescription* attributes,
		uint32_t descriptor_set_layout_count,
		VkDescriptorSetLayout* descriptor_set_layouts,
		uint32_t shader_stage_count,
		VkPipelineShaderStageCreateInfo* shader_stages,
		uint32_t push_constant_count,
		VkPushConstantRange* push_constant_ranges,
		VkViewport viewport,
		VkRect2D scissor,
		bool is_wireframe
	);

	Pipeline(Pipeline&& other);

	Pipeline(const Pipeline&) = delete; // Prevent copies.

	~Pipeline();

	operator VkPipeline() const;

	Pipeline& operator = (const Pipeline&) = delete; // Prevent copies.

	void bind(const CommandBuffer& command_buffer, VkPipelineBindPoint bind_point);

	VkPipelineLayout get_layout() const;

private:
	VkPipeline handle;
	VkPipelineLayout pipeline_layout;

	const Device& device;
};

}
