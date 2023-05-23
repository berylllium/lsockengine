#pragma once

#include <string>

#include <vulkan/vulkan.h>

#include "renderer/device.hpp"
#include "definitions.hpp"

namespace lise
{

class ShaderStage
{
public:
	ShaderStage(const Device& device, std::string path, VkShaderStageFlagBits shader_stage_flags);

	ShaderStage(ShaderStage&& other);

	ShaderStage(const ShaderStage&) = delete; // Prevent copies.

	~ShaderStage();

	ShaderStage& operator = (const ShaderStage&) = delete; // Prevent copies.

	VkShaderModule get_module() const;

	VkPipelineShaderStageCreateInfo get_pipeline_shader_stage_creation_info() const;

private:
	VkShaderModule module_handle;
	VkPipelineShaderStageCreateInfo shader_stage_create_info;

	const Device& device;
};

}
