#include "renderer/resource/shader_stage.hpp"

#include <fstream>
#include <memory>

#include <simple-logger.hpp>

namespace lise
{

std::unique_ptr<ShaderStage> ShaderStage::create(
	const Device* device,
	std::string path,
	vk::ShaderStageFlagBits shader_stage
)
{
	auto out = std::make_unique<ShaderStage>();

	// Copy trivial data.
	out->device = device;
	
	// Open file
	std::ifstream file(path, std::ios::binary | std::ios::in | std::ios::ate);

	if (file.fail())
	{
		sl::log_error("Unable to open shader bytecode file for shader `{}`.", path);
		return nullptr;
	}

	// Read the bytecode
	std::streampos size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::unique_ptr<char[]> file_buffer = std::make_unique<char[]>(size);
	file.read(file_buffer.get(), size);

	file.close();

	vk::ShaderModuleCreateInfo shader_module_ci(
		{},
		size,
		(uint32_t*) file_buffer.get()
	);

	vk::Result r;

	std::tie(r, out->module_handle) = device->logical_device.createShaderModule(shader_module_ci);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to create shader module for shader `{}`.", path);
		return nullptr;
	}

	out->shader_stage_create_info = vk::PipelineShaderStageCreateInfo(
		{},
		shader_stage,
		out->module_handle,
		"main"
	);

	return out;
}


ShaderStage::~ShaderStage()
{
	if (module_handle)
	{
		device->logical_device.destroy(module_handle);
	}
}

}
