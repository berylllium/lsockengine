#pragma once

#include <vulkan/vulkan.h>

#include "definitions.h"

typedef struct lise_shader_stage
{
	VkShaderModule module_handle;
	VkPipelineShaderStageCreateInfo shader_stage_create_info;
} lise_shader_stage;

bool lise_shader_stage_create(
	VkDevice device,
	const char* path,
	VkShaderStageFlagBits shader_stage_flags,
	lise_shader_stage* out_shader_stage
);

bool lise_shader_stage_destroy(VkDevice device, lise_shader_stage* shader_stage);
