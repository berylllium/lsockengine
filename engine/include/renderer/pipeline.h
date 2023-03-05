#pragma once

#include <vulkan/vulkan.h>

#include "definitions.h"
#include "renderer/render_pass.h"

typedef struct lise_pipeline
{
	VkPipeline handle;
	VkPipelineLayout pipeline_layout;
} lise_pipeline;

bool lise_pipeline_create(
	VkDevice device,
	lise_render_pass* render_pass,
	uint32_t attribute_count,
	VkVertexInputAttributeDescription* attributes,
	uint32_t descriptor_set_layout_count,
	VkDescriptorSetLayout* descriptor_set_layouts,
	uint32_t shader_stage_count,
	VkPipelineShaderStageCreateInfo* shader_stages,
	VkViewport viewport,
	VkRect2D scissor,
	bool is_wireframe,
	lise_pipeline* out_pipeline
);

void lise_pipeline_destroy(VkDevice device, lise_pipeline* pipeline);

void lise_pipeline_bind(lise_command_buffer* command_buffer, VkPipelineBindPoint bind_point, lise_pipeline* pipeline);
