#include "renderer/shader/object_shader.h"

#include "core/logger.h"
#include "renderer/pipeline.h"
#include "math/vector3.h"

#define ATTRIBUTE_COUNT 1

bool lise_object_shader_create(
	VkDevice device,
	lise_render_pass* render_pass,
	uint32_t framebuffer_width,
	uint32_t framebuffer_height,
	lise_object_shader* out_object_shader
)
{
	lise_shader_stage shader_stages[LOBJECT_SHADER_STAGE_COUNT];

	if (!lise_shader_stage_create(
		device,
		"assets/shaders/builtin.object_shader.vert.spv",
		VK_SHADER_STAGE_VERTEX_BIT,
		&shader_stages[0]
	))
	{
		LERROR("Failed to open the vertex shader binary file for the object_shader");
		return false;
	}

	if (!lise_shader_stage_create(
		device,
		"assets/shaders/builtin.object_shader.frag.spv",
		VK_SHADER_STAGE_FRAGMENT_BIT,
		&shader_stages[1]
	))
	{
		LERROR("Failed to open the fragment shader binary file for the object_shader");
		return false;
	}

	VkPipelineShaderStageCreateInfo shader_stage_create_infos[LOBJECT_SHADER_STAGE_COUNT] = {
		shader_stages[0].shader_stage_create_info,
		shader_stages[1].shader_stage_create_info
	};

	// Pipeline creation
	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = (float) framebuffer_height;
	viewport.width = (float) framebuffer_width;
	viewport.height = -(float) framebuffer_height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Scissor
	VkRect2D scissor;
	scissor.offset.x = scissor.offset.y = 0;
	scissor.extent.width = framebuffer_width;
	scissor.extent.height = framebuffer_height;

	// Attributes
	uint32_t offset = 0;
	VkVertexInputAttributeDescription attribute_descriptions[ATTRIBUTE_COUNT];

	// Position
	VkFormat formats[ATTRIBUTE_COUNT] = {
		VK_FORMAT_R32G32B32_SFLOAT
	};
	uint64_t sizes[ATTRIBUTE_COUNT] = {
		sizeof(lise_vec3)
	};
	for (uint32_t i = 0; i < ATTRIBUTE_COUNT; ++i)
	{
		attribute_descriptions[i].binding = 0;   // binding index - should match binding desc
		attribute_descriptions[i].location = i;  // attrib location
		attribute_descriptions[i].format = formats[i];
		attribute_descriptions[i].offset = offset;
		offset += sizes[i];
	}

	if (!lise_pipeline_create(
		device,
		render_pass,
		ATTRIBUTE_COUNT,
		attribute_descriptions,
		0,
		NULL,
		LOBJECT_SHADER_STAGE_COUNT,
		shader_stage_create_infos,
		viewport,
		scissor,
		false,
		&out_object_shader->pipeline
	))
	{
		LERROR("Failed to create graphics pipeline for shader builtin.object_shader");
		return false;
	}

	lise_shader_stage_destroy(device, &shader_stages[0]);
	lise_shader_stage_destroy(device, &shader_stages[1]);

	return true;
}

void lise_object_shader_destroy(VkDevice device, lise_object_shader* object_shader)
{
	lise_pipeline_destroy(device, &object_shader->pipeline);
}

void lise_object_shader_use(VkDevice device, lise_object_shader* object_shader)
{

}
