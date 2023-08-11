#pragma once

#include <string>

#include "renderer/device.hpp"
#include "definitions.hpp"

namespace lise
{

struct ShaderStage
{
	vk::ShaderModule module_handle;
	vk::PipelineShaderStageCreateInfo shader_stage_create_info;

	const Device* device;

	ShaderStage() = default;

	ShaderStage(const ShaderStage&) = delete; // Prevent copies.

	~ShaderStage();

	ShaderStage& operator = (const ShaderStage&) = delete; // Prevent copies.
	
	LAPI static std::unique_ptr<ShaderStage> create(
		const Device* device,
		std::string path,
		vk::ShaderStageFlagBits shader_stage
	);
};

}
