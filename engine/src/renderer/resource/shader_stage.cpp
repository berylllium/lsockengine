#include "renderer/resource/shader_stage.hpp"

#include <fstream>
#include <memory>

#include "core/logger.hpp"

namespace lise
{

ShaderStage::ShaderStage(const Device& device, std::string path, VkShaderStageFlagBits shader_stage_flags)
	: device(device)
{
	// Open file
	std::ifstream file(path, std::ios::binary | std::ios::in | std::ios::ate);

	if (file.fail())
	{
		LERROR("Unable to open shader bytecode file for shader `%s`.", path);
		throw std::exception();
	}

	// Read the bytecode
	std::streampos size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::unique_ptr<char[]> file_buffer = std::make_unique<char[]>(size);
	file.read(file_buffer.get(), size);

	file.close();

	VkShaderModuleCreateInfo shader_module_ci = {};
	shader_module_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_ci.codeSize = size;
	shader_module_ci.pCode = (uint32_t*) file_buffer.get();

	if (vkCreateShaderModule(device, &shader_module_ci, NULL, &module_handle) != VK_SUCCESS)
	{
		LERROR("Failed to create shader module for shader `%s`.", path);

		throw std::exception();
	}

	shader_stage_create_info = {};
	shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_create_info.stage = shader_stage_flags;
	shader_stage_create_info.module = module_handle;
	shader_stage_create_info.pName = "main";
}

ShaderStage::ShaderStage(ShaderStage&& other) : device(other.device)
{
	module_handle = other.module_handle;
	other.module_handle = nullptr;

	shader_stage_create_info = other.shader_stage_create_info;
	other.shader_stage_create_info = {};
}

ShaderStage::~ShaderStage()
{
	if (module_handle)
	{
		vkDestroyShaderModule(device, module_handle, NULL);
	}
}

VkShaderModule ShaderStage::get_module() const
{
	return module_handle;
}

VkPipelineShaderStageCreateInfo ShaderStage::get_pipeline_shader_stage_creation_info() const
{
	return shader_stage_create_info;
}

}
