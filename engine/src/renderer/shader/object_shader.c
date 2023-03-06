#include "renderer/shader/object_shader.h"

#include <stdlib.h>

#include "core/logger.h"
#include "renderer/pipeline.h"
#include "math/vector3.h"

#define ATTRIBUTE_COUNT 1

bool lise_object_shader_create(
	VkDevice device,
	VkPhysicalDeviceMemoryProperties memory_properties,
	lise_render_pass* render_pass,
	uint32_t framebuffer_width,
	uint32_t framebuffer_height,
	uint32_t swapchain_image_count,
	lise_object_shader* out_object_shader
)
{
	out_object_shader->global_descriptor_set_count = swapchain_image_count;

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

	// Global Descriptors
	VkDescriptorSetLayoutBinding global_ubo_layout_binding = {};
	global_ubo_layout_binding.binding = 0;
	global_ubo_layout_binding.descriptorCount = 1;
	global_ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	global_ubo_layout_binding.pImmutableSamplers = NULL;
	global_ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo global_layout_ci = {};
	global_layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	global_layout_ci.bindingCount = 1;
	global_layout_ci.pBindings = &global_ubo_layout_binding;

	if (vkCreateDescriptorSetLayout(device, &global_layout_ci, NULL, &out_object_shader->global_descriptor_set_layout)
		!= VK_SUCCESS
	)
	{
		LERROR("Failed to create descriptor set layout.");
		return false;
	}

	// Global descriptor pool
	VkDescriptorPoolSize global_pool_size = {};
	global_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	global_pool_size.descriptorCount = swapchain_image_count;

	VkDescriptorPoolCreateInfo global_pool_ci = {};
	global_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	global_pool_ci.poolSizeCount = 1;
	global_pool_ci.pPoolSizes = &global_pool_size;
	global_pool_ci.maxSets = swapchain_image_count;

	if (vkCreateDescriptorPool(device, &global_pool_ci, NULL, &out_object_shader->global_descriptor_pool) != VK_SUCCESS)
	{
		LERROR("Failed to create descriptor pool.");
		return false;
	}

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

	const int32_t descriptor_set_layout_count = 1;
	VkDescriptorSetLayout layouts[1] = {
		out_object_shader->global_descriptor_set_layout
	};

	if (!lise_pipeline_create(
		device,
		render_pass,
		ATTRIBUTE_COUNT,
		attribute_descriptions,
		descriptor_set_layout_count,
		layouts,
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

	// Create the uniform buffer
	if (!lise_vulkan_buffer_create(
		device,
		memory_properties,
		sizeof(lise_object_shader_global_ubo) * swapchain_image_count,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		true,
		&out_object_shader->global_uniform_buffer
	))
	{
		LERROR("Failed to create the global uniform buffer");
		return false;
	}

	// Allocate the global descriptor sets
	VkDescriptorSetLayout* global_layouts = malloc(sizeof(VkDescriptorSetLayout) * swapchain_image_count);

	for (uint32_t i = 0; i < swapchain_image_count; i++)
	{
		global_layouts[i] = out_object_shader->global_descriptor_set_layout;
	}

	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = out_object_shader->global_descriptor_pool;
	alloc_info.descriptorSetCount = swapchain_image_count;
	alloc_info.pSetLayouts = global_layouts;

	out_object_shader->global_descriptor_sets = malloc(sizeof(VkDescriptorSet) * swapchain_image_count);
	if (vkAllocateDescriptorSets(device, &alloc_info, out_object_shader->global_descriptor_sets) != VK_SUCCESS)
	{
		LERROR("Failed to allocate descriptor sets.");
		return false;
	}
	
	// Free the layouts
	free(global_layouts);

	// Set all global descriptor sets to be not updated
	out_object_shader->global_descriptor_sets_updated = calloc(swapchain_image_count, sizeof(bool));

	// Destroy the stages and modules as we do not need them after pipeline creation.
	lise_shader_stage_destroy(device, &shader_stages[0]);
	lise_shader_stage_destroy(device, &shader_stages[1]);

	return true;
}

void lise_object_shader_destroy(VkDevice device, lise_object_shader* object_shader)
{
	lise_vulkan_buffer_destroy(device, &object_shader->global_uniform_buffer);

	lise_pipeline_destroy(device, &object_shader->pipeline);

	vkDestroyDescriptorPool(device, object_shader->global_descriptor_pool, NULL);
	free(object_shader->global_descriptor_sets);
	free(object_shader->global_descriptor_sets_updated);

	vkDestroyDescriptorSetLayout(device, object_shader->global_descriptor_set_layout, NULL);
}

void lise_object_shader_use(uint32_t image_index, VkCommandBuffer command_buffer, lise_object_shader* object_shader)
{
	vkCmdBindDescriptorSets(
		command_buffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		object_shader->pipeline.pipeline_layout,
		0,
		1,
		&object_shader->global_descriptor_sets[image_index],
		0,
		0
	);

	lise_pipeline_bind(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, &object_shader->pipeline);
}

void lise_object_shader_set_global_ubo(lise_object_shader_global_ubo new_global_ubo, lise_object_shader* object_shader)
{
	object_shader->global_ubo = new_global_ubo;
	
	for (uint32_t i = 0; i < object_shader->global_descriptor_set_count; i++)
	{
		object_shader->global_descriptor_sets_updated[i] = false;
	}
}

void lise_object_shader_update_global_state(
	VkDevice device,
	lise_object_shader* object_shader,
	uint32_t image_index
)
{
	// Descriptor set info
	uint32_t size = sizeof(lise_object_shader_global_ubo);
	uint64_t offset = sizeof(lise_object_shader_global_ubo) * image_index;

	// Copy data
	lise_vulkan_buffer_load_data(
		device,
		&object_shader->global_uniform_buffer,
		offset,
		size,
		0,
		&object_shader->global_ubo
	);

	VkDescriptorBufferInfo buffer_info;
	buffer_info.buffer = object_shader->global_uniform_buffer.handle;
	buffer_info.offset = offset;
	buffer_info.range = size;

	// Update descriptor
	VkWriteDescriptorSet descriptor_write = {};
	descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_write.dstSet = object_shader->global_descriptor_sets[image_index];
	descriptor_write.dstBinding = 0;
	descriptor_write.dstArrayElement = 0;
	descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptor_write.descriptorCount = 1;
	descriptor_write.pBufferInfo = &buffer_info;

	vkUpdateDescriptorSets(device, 1, &descriptor_write, 0, 0);

	object_shader->global_descriptor_sets_updated[image_index] = true;
}
