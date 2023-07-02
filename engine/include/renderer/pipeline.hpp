#pragma once

#include <vector>

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
		uint32_t vertex_input_stride,
		std::vector<VkVertexInputAttributeDescription>& attributes,
		std::vector<VkDescriptorSetLayout>& descriptor_set_layouts,
		std::vector<VkPipelineShaderStageCreateInfo>& shader_stages,
		std::vector<VkPushConstantRange>& push_constant_ranges,
		VkViewport viewport,
		VkRect2D scissor,
		bool is_wireframe,
		bool depth_test_enabled
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
