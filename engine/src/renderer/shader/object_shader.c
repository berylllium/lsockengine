#include "renderer/shader/object_shader.h"

#include <stdlib.h>
#include <string.h>

#include "core/logger.h"
#include "renderer/pipeline.h"
#include "math/vertex.h"
#include "renderer/system/texture_system.h"

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
	out_object_shader->descriptor_set_count = swapchain_image_count;

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
		LERROR("Failed to create global descriptor set layout.");
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
		LERROR("Failed to create global descriptor pool.");
		return false;
	}

	// Object descriptors.
	VkDescriptorType local_descriptor_types[LOBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT] = {
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
	};

	VkDescriptorSetLayoutBinding local_bindings[LOBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT] = { 0 };

	for (uint32_t i = 0; i < LOBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT; i++)
	{
		local_bindings[i].binding = i;
		local_bindings[i].descriptorCount = 1;
		local_bindings[i].descriptorType = local_descriptor_types[i];
		local_bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	}

	VkDescriptorSetLayoutCreateInfo local_descriptor_set_layout_ci = {};
	local_descriptor_set_layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	local_descriptor_set_layout_ci.bindingCount = LOBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT;
	local_descriptor_set_layout_ci.pBindings = local_bindings;

	if (vkCreateDescriptorSetLayout(
			device,
			&local_descriptor_set_layout_ci,
			NULL,
			&out_object_shader->object_descriptor_set_layout
		) != VK_SUCCESS
	)
	{
		LERROR("Failed to create object descriptor set layout.");
		return false;
	}

	// Local descriptor pool.
	VkDescriptorPoolSize object_descriptor_pool_sizes[LOBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT];
	// Uniform buffers
	object_descriptor_pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	object_descriptor_pool_sizes[0].descriptorCount = LOBJECT_SHADER_MAX_OBJECT_COUNT;
	// Image samplers
	object_descriptor_pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	object_descriptor_pool_sizes[1].descriptorCount =
		LOBJECT_SHADER_SAMPLER_PER_OBJECT_COUNT * LOBJECT_SHADER_MAX_OBJECT_COUNT;

	VkDescriptorPoolCreateInfo object_descriptor_pool_ci = {};
	object_descriptor_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	object_descriptor_pool_ci.poolSizeCount = LOBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT;
	object_descriptor_pool_ci.pPoolSizes = object_descriptor_pool_sizes;
	object_descriptor_pool_ci.maxSets = LOBJECT_SHADER_MAX_OBJECT_COUNT;

	if (vkCreateDescriptorPool(device, &object_descriptor_pool_ci, NULL, &out_object_shader->object_descriptor_pool)
		!= VK_SUCCESS
	)
	{
		LERROR("Failed to create object descriptor pool.");
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
#define ATTRIBUTE_COUNT 2
	VkVertexInputAttributeDescription attribute_descriptions[ATTRIBUTE_COUNT];

	// Position
	VkFormat formats[ATTRIBUTE_COUNT] = {
		VK_FORMAT_R32G32B32_SFLOAT,
		VK_FORMAT_R32G32_SFLOAT
	};
	uint64_t sizes[ATTRIBUTE_COUNT] = {
		sizeof(lise_vec3),
		sizeof(lise_vec2)
	};
	for (uint32_t i = 0; i < ATTRIBUTE_COUNT; ++i)
	{
		attribute_descriptions[i].binding = 0;   // binding index - should match binding desc
		attribute_descriptions[i].location = i;  // attrib location
		attribute_descriptions[i].format = formats[i];
		attribute_descriptions[i].offset = offset;
		offset += sizes[i];
	}

	const int32_t descriptor_set_layout_count = 2;
	VkDescriptorSetLayout layouts[2] = {
		out_object_shader->global_descriptor_set_layout,
		out_object_shader->object_descriptor_set_layout
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

	// Create the global uniform buffer
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

	// Create the object uniform buffer.
	if (!lise_vulkan_buffer_create(
		device,
		memory_properties,
		sizeof(lise_object_shader_object_ubo) * LOBJECT_SHADER_MAX_OBJECT_COUNT,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		true,
		&out_object_shader->object_uniform_buffer
	))
	{
		LERROR("Failed to create the object unbiform buffer.");
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
	lise_vulkan_buffer_destroy(device, &object_shader->object_uniform_buffer);
	lise_vulkan_buffer_destroy(device, &object_shader->global_uniform_buffer);

	lise_pipeline_destroy(device, &object_shader->pipeline);

	vkDestroyDescriptorPool(device, object_shader->object_descriptor_pool, NULL);
	vkDestroyDescriptorSetLayout(device, object_shader->object_descriptor_set_layout, NULL);

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
	
	for (uint32_t i = 0; i < object_shader->descriptor_set_count; i++)
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

bool lise_object_shader_register_object(
	lise_object_shader* object_shader, 
	VkDevice device,
	uint32_t swapchain_image_count,
	lise_object_shader_object* out_object
)
{
	lise_object_shader_object object = {};

	object.ub_index = object_shader->current_uniform_buffer_index++;

	object.descriptor_sets = malloc(swapchain_image_count * sizeof(VkDescriptorSet));
	object.is_descriptor_dirty = malloc(LOBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT * swapchain_image_count * sizeof(bool));

	object.descriptor_set_count = swapchain_image_count;

	// Set all descriptors to be dirty.
	for (uint32_t i = 0; i < LOBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT * swapchain_image_count; i++)
	{
		object.is_descriptor_dirty[i] = true;
	}

	// Allocate descriptor sets.
	VkDescriptorSetLayout* layouts = malloc(swapchain_image_count * sizeof(VkDescriptorSetLayout));

	for (uint32_t i = 0; i < swapchain_image_count; i++)
	{
		layouts[i] = object_shader->object_descriptor_set_layout;
	}

	VkDescriptorSetAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocate_info.descriptorPool = object_shader->object_descriptor_pool;
	allocate_info.descriptorSetCount = swapchain_image_count;
	allocate_info.pSetLayouts = layouts;

	VkResult r = vkAllocateDescriptorSets(device, &allocate_info, object.descriptor_sets);

	free(layouts);

	if (r != VK_SUCCESS)
	{
		LERROR("Failed to allocate object descriptor sets.");
		free(object.descriptor_sets);
		free(object.is_descriptor_dirty);
		return false;
	}
	
	object.diffuse_texture = lise_texture_system_get_default_texture();

	*out_object = object;

	return true;
}

bool lise_object_shader_free_object(
	VkDevice device,
	lise_object_shader* object_shader,
	lise_object_shader_object* object
)
{
	vkFreeDescriptorSets(device, object_shader->object_descriptor_pool, 2, object->descriptor_sets);

	free(object->descriptor_sets);
	free(object->is_descriptor_dirty);

	object_shader->current_uniform_buffer_index--;
}

void lise_object_shader_set_object_data(
	lise_object_shader_object* object,
	lise_object_shader_object_ubo* object_ubo,
	lise_texture* diffuse_texture
)
{
	if (object_ubo)
	{
		object->object_ubo = *object_ubo;

		for (uint32_t i = 0; i < object->descriptor_set_count; i++)
		{
			
			object->is_descriptor_dirty[LOBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT * i + 0] = true;
		}
	}

	if (diffuse_texture)
	{
		object->diffuse_texture = diffuse_texture;

		for (uint32_t i = 0; i < object->descriptor_set_count; i++)
		{
			object->is_descriptor_dirty[LOBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT * i + 1] = true;
		}
	}
}

void lise_object_shader_update_object(
	lise_object_shader_object* object,
	lise_object_shader* object_shader,
	VkCommandBuffer command_buffer,
	VkDevice device,
	uint32_t swapchain_image_index
)
{
	VkWriteDescriptorSet descriptor_writes[LOBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT];
	uint32_t descriptor_write_count = 0;

	// Uniform buffer.
	if (object->is_descriptor_dirty[swapchain_image_index * LOBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT + 0])
	{
		// Upload to buffer.
		lise_vulkan_buffer_load_data(
			device,
			&object_shader->object_uniform_buffer,
			object->ub_index * sizeof(lise_object_shader_object_ubo),
			sizeof(lise_object_shader_object_ubo),
			0,
			&object->object_ubo
		);

		// Uniform buffer is out of date; update.
		VkDescriptorBufferInfo* buffer_info = alloca(sizeof(VkDescriptorBufferInfo));
		buffer_info->buffer = object_shader->object_uniform_buffer.handle;
		buffer_info->offset = sizeof(lise_object_shader_object_ubo) * object->ub_index;
		buffer_info->range = sizeof(lise_object_shader_object_ubo);

		VkWriteDescriptorSet ubo_write = {};
		ubo_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		ubo_write.dstSet = object->descriptor_sets[swapchain_image_index];
		ubo_write.dstBinding = 0;
		ubo_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo_write.descriptorCount = 1;
		ubo_write.pBufferInfo = buffer_info;

		descriptor_writes[0] = ubo_write;
		descriptor_write_count++;

		// Set dirty to false.
		object->is_descriptor_dirty[swapchain_image_index * LOBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT + 0] = false;
	}

	// Texture sampler.
	if (object->is_descriptor_dirty[swapchain_image_index * LOBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT + 1])
	{
		VkDescriptorImageInfo* image_info = alloca(sizeof(VkDescriptorImageInfo));
		image_info->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_info->imageView = object->diffuse_texture->image.image_view;
		image_info->sampler = object->diffuse_texture->sampler;

		VkWriteDescriptorSet d_write = {};
		d_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		d_write.dstSet = object->descriptor_sets[swapchain_image_index];
		d_write.dstBinding = 1;
		d_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		d_write.descriptorCount = 1;
		d_write.pImageInfo = image_info;

		descriptor_writes[1] = d_write;
		descriptor_write_count++;

		// Set dirty to false.
		object->is_descriptor_dirty[swapchain_image_index * LOBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT + 1] = false;
	}

	if (descriptor_write_count > 0)
	{
		vkUpdateDescriptorSets(device, descriptor_write_count, descriptor_writes, 0, NULL);
	}
}
