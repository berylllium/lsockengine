#include "renderer/shader_stage.h"

#include <stdlib.h>
#include <string.h>

#include "core/logger.h"
#include "platform/filesystem.h"

bool lise_shader_stage_create(
	VkDevice device,
	const char* path,
	VkShaderStageFlagBits shader_stage_flags,
	lise_shader_stage* out_shader_stage
)
{
	memset(out_shader_stage, 0, sizeof(lise_shader_stage));

	// Open file
	lise_file_handle file_handle;
	if (!lise_filesystem_open(path, LISE_FILE_MODE_READ, true, &file_handle))
	{
		LERROR("Unable to open shader bytecode file for shader `%s`.", path);
		return false;
	}

	// Read the bytecode
	uint64_t size = 0;
	char* file_buffer = NULL;

	if (!lise_filesystem_read_all(&file_handle, &size, &file_buffer))
	{
		LERROR("Failed to read shader bytecode for shader `%s`.", path);
		return false;
	}

	lise_filesystem_close(&file_handle);

	VkShaderModuleCreateInfo shader_module_ci = {};
	shader_module_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_ci.codeSize = size;
	shader_module_ci.pCode = (uint32_t*) file_buffer;

	if (vkCreateShaderModule(device, &shader_module_ci, NULL, &out_shader_stage->module_handle) != VK_SUCCESS)
	{
		LERROR("Failed to create shader module for shader `%s`.", path);

		free(file_buffer);

		return false;
	}

	free(file_buffer);

	out_shader_stage->shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	out_shader_stage->shader_stage_create_info.stage = shader_stage_flags;
	out_shader_stage->shader_stage_create_info.module = out_shader_stage->module_handle;
	out_shader_stage->shader_stage_create_info.pName = "main";

	return true;
}

bool lise_shader_stage_destroy(VkDevice device, lise_shader_stage* shader_stage)
{
	vkDestroyShaderModule(device, shader_stage->module_handle, NULL);

	shader_stage->module_handle = NULL;
}
