#include "renderer/shader.h"

#include <stdlib.h>
#include <string.h>

#include <container/darray.h>

#include "core/logger.h"
#include "loader/shader_config_loader.h"
#include "renderer/shader_stage.h"
#include "renderer/system/texture_system.h"

#define align(x, n) (((x - 1) | (n - 1)) + 1)

static lise_shader_uniform_type parse_uniform_type(const char* type);
static uint64_t get_uniform_type_size(lise_shader_uniform_type type);

static VkFormat parse_attribute_format(const char* type);
static uint64_t get_attribute_format_size(VkFormat format);

bool lise_shader_create(
	VkDevice device,
	VkPhysicalDeviceMemoryProperties memory_properties,
	uint64_t minimum_uniform_alignment,
	const char* path,
	const lise_render_pass* render_pass, // TODO: Temporary, add a render pass system.
	uint32_t framebuffer_width,
	uint32_t framebuffer_height,
	uint32_t swapchain_image_count,
	lise_shader* out_shader
)
{
	// Zero out the out_shader memory to not mess up any mid-function calls to lise_shader_destroy.
	memset(out_shader, 0, sizeof(lise_shader));

	// Define macro for freeing the dynamic resources used in the function. Just so the code isn't so cluttered.
	#define FREE_F \
		lise_shader_destroy(device, out_shader); \
		for (uint32_t i = 0; i < shader_config.stage_count; i++) \
		{ \
			lise_shader_stage_destroy(device, &shader_stages[i]); \
		} \
		free(shader_stages); \
		free(shader_stage_cis); \
		lise_shader_config_free(&shader_config);

	// Retrieve the shader config.
	lise_shader_config shader_config;

	if (!lise_shader_config_load(path, &shader_config))
	{
		LERROR("Failed to load shader config file for `%s`.", path);

		return false;
	}

	// Copy over trivial data.
	out_shader->name = strdup(shader_config.name);
	out_shader->swapchain_image_count = swapchain_image_count;
	out_shader->minimum_uniform_alignment = minimum_uniform_alignment;

	// Create the shader stages.
	lise_shader_stage* shader_stages = calloc(shader_config.stage_count, sizeof(lise_shader_stage));

	VkPipelineShaderStageCreateInfo* shader_stage_cis =
		malloc(shader_config.stage_count * sizeof(VkPipelineShaderStageCreateInfo));

	for (uint64_t i = 0; i < shader_config.stage_count; i++)
	{
		VkShaderStageFlagBits stage_flag;

		if (strcmp(shader_config.stage_names[i], "vertex") == 0)
		{
			stage_flag = VK_SHADER_STAGE_VERTEX_BIT;
		}
		else if (strcmp(shader_config.stage_names[i], "fragment") == 0)
		{
			stage_flag = VK_SHADER_STAGE_FRAGMENT_BIT;
		}
		else
		{
			LERROR("Stage name provided in config `%s`, `%s` is not valid.", path, shader_config.stage_names[i]);

			FREE_F
			return false;
		}
		
		if (!lise_shader_stage_create(device, shader_config.stage_file_names[i], stage_flag, &shader_stages[i]))
		{
			LERROR("Failed to open shader binary file for config file `%s`.", path);

			FREE_F
			return false;
		}

		shader_stage_cis[i] = shader_stages[i].shader_stage_create_info;
	}

	// Sort the uniforms.
	bool global_has_uniform = false;
	bool global_has_sampler = false;
	uint64_t global_uniform_total_size = 0;
	blib_darray global_uniforms = blib_darray_create(lise_shader_uniform);
	blib_darray global_samplers = blib_darray_create(lise_shader_uniform);

	bool instance_has_uniform = false;
	bool instance_has_sampler = false;
	uint64_t instance_uniform_total_size = 0;
	blib_darray instance_uniforms = blib_darray_create(lise_shader_uniform);
	blib_darray instance_samplers = blib_darray_create(lise_shader_uniform);

	uint64_t local_uniform_total_size = 0;
	blib_darray local_uniforms = blib_darray_create(lise_shader_uniform);

	for (uint64_t i = 0; i < shader_config.uniform_count; i++)
	{
		lise_shader_uniform uniform;

		uniform.type = parse_uniform_type(shader_config.uniforms[i].type);
		uniform.scope = shader_config.uniforms[i].scope;
		uniform.name = strdup(shader_config.uniforms[i].name);

		uniform.size = get_uniform_type_size(uniform.type);


		switch (uniform.scope)
		{
		case LISE_SHADER_SCOPE_GLOBAL:
			uniform.offset = global_uniform_total_size;

			blib_darray_push_back(
				&global_uniforms,
				&uniform
			);

			if (uniform.type == LISE_SHADER_UNIFORM_TYPE_SAMPLER)
			{
				global_has_sampler = true;
			}
			else
			{
				global_has_uniform = true;
			}

			global_uniform_total_size += uniform.size;
			break;
		case LISE_SHADER_SCOPE_INSTANCE:
			uniform.offset = instance_uniform_total_size;

			blib_darray_push_back(
				uniform.type == LISE_SHADER_UNIFORM_TYPE_SAMPLER ? &instance_samplers : &instance_uniforms,
				&uniform
			);
			
			if (uniform.type == LISE_SHADER_UNIFORM_TYPE_SAMPLER)
			{
				instance_has_sampler = true;
			}
			else
			{
				instance_has_uniform = true;
			}

			instance_uniform_total_size += uniform.size;
			break;
		case LISE_SHADER_SCOPE_LOCAL:
			uniform.offset = local_uniform_total_size;

			blib_darray_push_back(
				&local_uniforms,
				&uniform
			);

			local_uniform_total_size += uniform.size;
			break;
		}
	}

	out_shader->instance_ubo_size = instance_uniform_total_size;
	out_shader->global_ubo_size = global_uniform_total_size;

	out_shader->instance_ubo_stride = align(instance_uniform_total_size, minimum_uniform_alignment);
	out_shader->global_ubo_stride = align(global_uniform_total_size, minimum_uniform_alignment);

	// Copy over uniforms to out_shader.
	out_shader->global_uniform_count = global_uniforms.size;
	out_shader->global_uniforms = malloc(global_uniforms.size * sizeof(lise_shader_uniform));
	memcpy(out_shader->global_uniforms, global_uniforms.data, global_uniforms.size * sizeof(lise_shader_uniform));

	out_shader->instance_uniform_count = instance_uniforms.size;
	out_shader->instance_uniforms = malloc(instance_uniforms.size * sizeof(lise_shader_uniform));
	memcpy(out_shader->instance_uniforms, instance_uniforms.data, instance_uniforms.size * sizeof(lise_shader_uniform));

	out_shader->instance_sampler_count = instance_samplers.size;
	out_shader->instance_samplers = malloc(instance_samplers.size * sizeof(lise_shader_uniform));
	memcpy(out_shader->instance_samplers, instance_samplers.data, instance_samplers.size * sizeof(lise_shader_uniform));

	// Free the darrays.
	blib_darray_free(&global_uniforms);
	blib_darray_free(&instance_uniforms);
	blib_darray_free(&instance_samplers);

	// Create global descriptor set layout.
	blib_darray global_set_bindings = blib_darray_create(VkDescriptorSetLayoutBinding);

	if (global_has_uniform)
	{
		VkDescriptorSetLayoutBinding binding = {};
		binding.binding = global_set_bindings.size;
		binding.descriptorCount = 1;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // TODO: Make configurable.

		blib_darray_push_back(&global_set_bindings, &binding);
	}

	// TODO: Handle global samplers.

	VkDescriptorSetLayoutCreateInfo global_layout_ci = {};
	global_layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	global_layout_ci.bindingCount = global_set_bindings.size;
	global_layout_ci.pBindings = global_set_bindings.data;

	if (vkCreateDescriptorSetLayout(device, & global_layout_ci, NULL, &out_shader->global_descriptor_set_layout)
		!= VK_SUCCESS
	)
	{
		LERROR("Failed to create global descriptor set layout.");

		FREE_F
		return false;
	}

	blib_darray_free(&global_set_bindings);

	// Create global descriptor pool.
	VkDescriptorPoolSize global_pool_size;
	global_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	global_pool_size.descriptorCount = swapchain_image_count * swapchain_image_count;

	VkDescriptorPoolCreateInfo global_pool_ci = {};
	global_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	global_pool_ci.poolSizeCount = 1;
	global_pool_ci.pPoolSizes = &global_pool_size;
	global_pool_ci.maxSets = swapchain_image_count;

	if (vkCreateDescriptorPool(device, &global_pool_ci, NULL, &out_shader->global_descriptor_pool) != VK_SUCCESS)
	{
		LERROR("Failed to create global descriptor pool.");
		
		FREE_F
		return false;
	}

	// Create instance descriptor set layout.
	blib_darray instance_set_bindings = blib_darray_create(VkDescriptorSetLayoutBinding);

	if (instance_has_uniform)
	{
		VkDescriptorSetLayoutBinding binding = {};
		binding.binding = instance_set_bindings.size;
		binding.descriptorCount = 1;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // TODO: Make configurable.

		blib_darray_push_back(&instance_set_bindings, &binding);
	}
	
	if (instance_has_sampler)
	{
		VkDescriptorSetLayoutBinding binding = {};
		binding.binding = instance_set_bindings.size;
		binding.descriptorCount = 1;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // TODO: Make configurable.

		blib_darray_push_back(&instance_set_bindings, &binding);
	}

	VkDescriptorSetLayoutCreateInfo instance_layout_ci = {};
	instance_layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	instance_layout_ci.bindingCount = instance_set_bindings.size;
	instance_layout_ci.pBindings = blib_darray_get(&instance_set_bindings, 0);

	if (vkCreateDescriptorSetLayout(device, &instance_layout_ci, NULL, &out_shader->instance_descriptor_set_layout)
		!= VK_SUCCESS
	)
	{
		LERROR("Failed to create instance descriptor set layout.");

		FREE_F
		return false;
	}

	VkDescriptorPoolSize* instance_sizes = malloc(instance_set_bindings.size * sizeof(VkDescriptorPoolSize));

	for (uint64_t i = 0; i < instance_set_bindings.size; i++)
	{
		instance_sizes[i].type =
			((VkDescriptorSetLayoutBinding*) blib_darray_get(&instance_set_bindings, i))->descriptorType;
		instance_sizes[i].descriptorCount = LSHADER_MAX_INSTANCE_COUNT; // TODO: Add caluclation for exact dCount.
	}

	VkDescriptorPoolCreateInfo instance_pool_ci = {};
	instance_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	instance_pool_ci.poolSizeCount = instance_set_bindings.size;
	instance_pool_ci.pPoolSizes = instance_sizes;
	instance_pool_ci.maxSets = LSHADER_MAX_INSTANCE_COUNT;

	if (vkCreateDescriptorPool(device, &instance_pool_ci, NULL, &out_shader->instance_descriptor_pool) != VK_SUCCESS)
	{
		LERROR("Failed to create instance descriptor pool.");

		FREE_F
		return false;
	}

	blib_darray_free(&instance_set_bindings);

	// Push constants / Local uniforms.
	uint32_t push_constant_count = local_uniforms.size;
	VkPushConstantRange* push_constant_ranges = malloc(local_uniforms.size * sizeof(VkPushConstantRange));

	for (uint32_t i = 0; i < local_uniforms.size; i++)
	{
		lise_shader_uniform* pc = ((lise_shader_uniform*) blib_darray_get(&local_uniforms, i));
		push_constant_ranges[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // TODO: Make configurable.
		push_constant_ranges[i].size = pc->size;
		push_constant_ranges[i].offset = pc->offset;
	}

	blib_darray_free(&local_uniforms);

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

	// Vertex attributes
	VkVertexInputAttributeDescription* attribs =
		malloc(shader_config.attribute_count * sizeof(VkVertexInputAttributeDescription));
	
	out_shader->vertex_attribute_count = shader_config.attribute_count;
	out_shader->vertex_attributes = malloc(shader_config.attribute_count * sizeof(lise_shader_attribute));
	
	uint32_t offset = 0;
	for (uint32_t i = 0; i < shader_config.attribute_count; i++)
	{
		attribs[i].binding = 0;
		attribs[i].location = i;
		attribs[i].format = parse_attribute_format(shader_config.attributes[i].type);
		attribs[i].offset = offset;

		uint32_t attrib_size = get_attribute_format_size(attribs[i].format);

		offset += attrib_size;

		out_shader->vertex_attributes[i].name = strdup(shader_config.attributes[i].name);
		out_shader->vertex_attributes[i].type = attribs[i].format;
		out_shader->vertex_attributes[i].size = attrib_size;
	}

	// Create pipeline.
	VkDescriptorSetLayout set_layouts[2] = {
		out_shader->global_descriptor_set_layout,
		out_shader->instance_descriptor_set_layout
	};

	if (!lise_pipeline_create(
		device,
		render_pass,
		shader_config.attribute_count,
		attribs,
		2,
		set_layouts,
		shader_config.stage_count,
		shader_stage_cis,
		push_constant_count,
		push_constant_ranges,
		viewport,
		scissor,
		false,
		&out_shader->pipeline
	))
	{
		LERROR("Failed to create graphics pipeline.");

		FREE_F
		return false;
	}

	// Allocate the global uniform buffer object.
	if (!lise_vulkan_buffer_create(
		device,
		memory_properties,
		out_shader->global_ubo_stride * swapchain_image_count, // We use the stride, not the total size.
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		true,
		&out_shader->global_ub
	))
	{
		LERROR("Failed to allocate the global uniform buffer object.");

		FREE_F
		return false;
	}

	// Allocate global descriptor sets.
	VkDescriptorSetLayout* global_set_layouts = malloc(swapchain_image_count * sizeof(VkDescriptorSetLayout));

	for (uint32_t i = 0; i < swapchain_image_count; i++)
	{
		global_set_layouts[i] = out_shader->global_descriptor_set_layout;
	}

	VkDescriptorSetAllocateInfo global_set_allocate_info = {};
	global_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	global_set_allocate_info.descriptorPool = out_shader->global_descriptor_pool;
	global_set_allocate_info.descriptorSetCount = swapchain_image_count;
	global_set_allocate_info.pSetLayouts = global_set_layouts;

	out_shader->global_descriptor_sets = malloc(swapchain_image_count * sizeof(VkDescriptorSet));
	if (vkAllocateDescriptorSets(device, &global_set_allocate_info, out_shader->global_descriptor_sets) != VK_SUCCESS)
	{
		LERROR("Failed to allocate global descriptor sets.");

		FREE_F
		return false;
	}

	// Setup the descriptors to point to the corrosponding location in the global uniform buffer.
	VkDescriptorBufferInfo* global_descriptor_buffer_infos =
		malloc(swapchain_image_count * sizeof(VkDescriptorBufferInfo));

	VkWriteDescriptorSet* global_descriptor_writes = calloc(swapchain_image_count, sizeof(VkWriteDescriptorSet));

	for (uint32_t i = 0; i < swapchain_image_count; i++)
	{
		global_descriptor_buffer_infos[i].buffer = out_shader->global_ub.handle;
		global_descriptor_buffer_infos[i].offset = i * out_shader->global_ubo_stride;
		global_descriptor_buffer_infos[i].range = global_uniform_total_size;

		global_descriptor_writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		global_descriptor_writes[i].dstSet = out_shader->global_descriptor_sets[i];
		global_descriptor_writes[i].dstBinding = 0;
		global_descriptor_writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		global_descriptor_writes[i].descriptorCount = 1;
		global_descriptor_writes[i].pBufferInfo = &global_descriptor_buffer_infos[i];
	}

	vkUpdateDescriptorSets(device, swapchain_image_count, global_descriptor_writes, 0, NULL);

	free(global_descriptor_buffer_infos);
	free(global_descriptor_writes);

	// Set global ubos to be dirty.
	out_shader->global_ubo_dirty = malloc(swapchain_image_count * sizeof(bool));
	memset(out_shader->global_ubo_dirty, true, swapchain_image_count * sizeof(bool));

	// Update descriptors to point to the correct region within the global uniform buffer.
	VkWriteDescriptorSet* global_set_writes = malloc(swapchain_image_count * sizeof(VkWriteDescriptorSet));

	// Allocate the instance uniform buffer.
	if (!lise_vulkan_buffer_create(
		device,
		memory_properties,
		instance_uniform_total_size * LSHADER_MAX_INSTANCE_COUNT * swapchain_image_count,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		true,
		&out_shader->instance_ub
	))
	{
		LERROR("Failed to create the object unbiform buffer.");

		FREE_F
		return false;
	}

	// Allocate the instance uniform buffer object free list.
	out_shader->instance_ubo_free_list = malloc(LSHADER_MAX_INSTANCE_COUNT * sizeof(bool));

	// Set free state of all available slots to true.
	memset(out_shader->instance_ubo_free_list, true, LSHADER_MAX_INSTANCE_COUNT * sizeof(bool));

	// Free stuff.
	free(attribs);
	free(push_constant_ranges);
	free(global_set_layouts);

	for (uint32_t i = 0; i < shader_config.stage_count; i++)
	{
		lise_shader_stage_destroy(device, &shader_stages[i]);
	}

	free(shader_stage_cis);
	free(shader_stages);

	lise_shader_config_free(&shader_config);

	return true;
}

void lise_shader_destroy(VkDevice device, lise_shader* shader)
{
	// Free the uniform buffers.
	lise_vulkan_buffer_destroy(device, &shader->global_ub);
	lise_vulkan_buffer_destroy(device, &shader->instance_ub);
	free(shader->instance_ubo_free_list);
	free(shader->global_ubo_dirty);

	// Free the pipeline.
	lise_pipeline_destroy(device, &shader->pipeline);

	// Destroy the descriptor pools.
	vkDestroyDescriptorPool(device, shader->global_descriptor_pool, NULL);
	vkDestroyDescriptorPool(device, shader->instance_descriptor_pool, NULL);

	// Destroy the descriptor set layouts.
	vkDestroyDescriptorSetLayout(device, shader->global_descriptor_set_layout, NULL);
	vkDestroyDescriptorSetLayout(device, shader->instance_descriptor_set_layout, NULL);

	// Free remaining pointers.
	free(shader->name);

	free(shader->vertex_attributes);

	free(shader->global_uniforms);
	free(shader->instance_uniforms);
	free(shader->instance_samplers);
}

void lise_shader_use(lise_shader* shader, VkCommandBuffer command_buffer, uint32_t current_image)
{
	vkCmdBindDescriptorSets(
		command_buffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		shader->pipeline.pipeline_layout,
		0,
		1,
		&shader->global_descriptor_sets[current_image],
		0,
		NULL
	);

	lise_pipeline_bind(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
}

bool lise_shader_allocate_instance(VkDevice device, lise_shader* shader, lise_shader_instance* out_instance)
{
	// Iterate the free list and pick a free spot.
	bool slot_found = false;
	uint64_t id;

	uint32_t q = 0;

	for (uint64_t i = 0; i < LSHADER_MAX_INSTANCE_COUNT; i++)
	{
		if (shader->instance_ubo_free_list[i])
		{
			id = i;
			slot_found = true;
			break;
		}
	}

	if (!slot_found)
	{
		LERROR("Failed to find a free slot in the instance uniform buffer of shader `%s`.", shader->name);
		return false;
	}

	// Claim slot.
	shader->instance_ubo_free_list[id] = false;
	out_instance->id = id;

	// Allocate arrays.
	out_instance->descriptor_sets = malloc(shader->swapchain_image_count * sizeof(VkDescriptorSet));

	out_instance->ubo_dirty = malloc(shader->swapchain_image_count * sizeof(bool));

	if (shader->instance_sampler_count)
	{
		out_instance->samplers =
			malloc(shader->swapchain_image_count * shader->instance_sampler_count * sizeof(lise_texture*));

		out_instance->sampler_dirty =
			malloc(shader->swapchain_image_count * shader->instance_sampler_count * sizeof(bool));

		// Set samplers to default and dirty.
		for (uint32_t i = 0; i < shader->instance_sampler_count; i++)
		{
			out_instance->samplers[i] = lise_texture_system_get_default_texture();

			for (uint32_t j = 0; j < shader->swapchain_image_count; j++)
			{
				out_instance->sampler_dirty[i * shader->swapchain_image_count + j] = true;
			}
		}
	}

	// Set all uniform buffer objects to be dirty.
	memset(out_instance->ubo_dirty, true, shader->swapchain_image_count * sizeof(bool));

	// Allocate the descriptor sets.
	VkDescriptorSetLayout* layouts = malloc(shader->swapchain_image_count * sizeof(VkDescriptorSetLayout));
	for (uint32_t i = 0; i < shader->swapchain_image_count; i++)
	{
		layouts[i] = shader->instance_descriptor_set_layout;
	}

	VkDescriptorSetAllocateInfo d_set_ai = {};
	d_set_ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	d_set_ai.descriptorPool = shader->instance_descriptor_pool;
	d_set_ai.descriptorSetCount = shader->swapchain_image_count;
	d_set_ai.pSetLayouts = layouts;

	VkResult r = vkAllocateDescriptorSets(device, &d_set_ai, out_instance->descriptor_sets);

	free(layouts);

	if (r != VK_SUCCESS)
	{
		LERROR("Failed to allocate instance descriptor sets for shader `%s`.", shader->name);

		free(out_instance->descriptor_sets);
		free(out_instance->ubo_dirty);
		return false;
	}

	// Point the descriptors to the corrosponding location in the uniform buffer.
	VkDescriptorBufferInfo* instance_descriptor_buffer_infos =
		malloc(shader->swapchain_image_count * sizeof(VkDescriptorBufferInfo));

	VkWriteDescriptorSet* instance_descriptor_writes =
		calloc(shader->swapchain_image_count, sizeof(VkWriteDescriptorSet));

	for (uint32_t i = 0; i < shader->swapchain_image_count; i++)
	{
		instance_descriptor_buffer_infos[i].buffer = shader->instance_ub.handle;
		instance_descriptor_buffer_infos[i].offset =
			(shader->swapchain_image_count * id + i) * shader->instance_ubo_stride;
		instance_descriptor_buffer_infos[i].range = shader->instance_ubo_size;

		instance_descriptor_writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		instance_descriptor_writes[i].dstSet = out_instance->descriptor_sets[i];
		instance_descriptor_writes[i].dstBinding = 0;
		instance_descriptor_writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		instance_descriptor_writes[i].descriptorCount = 1;
		instance_descriptor_writes[i].pBufferInfo = &instance_descriptor_buffer_infos[i];
	}

	vkUpdateDescriptorSets(device, shader->swapchain_image_count, instance_descriptor_writes, 0, NULL);

	free(instance_descriptor_buffer_infos);
	free(instance_descriptor_writes);

	return true;
}

void lise_shader_free_instance(VkDevice device, lise_shader* shader, lise_shader_instance* instance)
{
	// Return slot to the free list.
	shader->instance_ubo_free_list[instance->id] = true;

	// Free descriptor sets.
	vkFreeDescriptorSets(
		device,
		shader->instance_descriptor_pool,
		shader->swapchain_image_count,
		instance->descriptor_sets
	);

	// Free remaining pointers.
	free(instance->descriptor_sets);

	free(instance->ubo_dirty);

	free(instance->samplers);
	free(instance->sampler_dirty);
}

void lise_shader_set_global_ubo(VkDevice device, lise_shader* shader, void* data)
{
	shader->global_ubo = data;
	
	// Set ubos to be dirty.
	memset(shader->global_ubo_dirty, true, shader->swapchain_image_count * sizeof(bool));
}

void lise_shader_update_global_uniforms(
	VkDevice device,
	lise_shader* shader,
	uint32_t current_image
)
{
	// Uniform buffers.
	if (shader->global_ubo_dirty[current_image])
	{
		lise_vulkan_buffer_load_data(
			device,
			&shader->global_ub,
			current_image * shader->global_ubo_stride,
			shader->global_ubo_size,
			0,
			shader->global_ubo
		);

		// Undirty ubo.
		shader->global_ubo_dirty[current_image] = false;
	}
}

void lise_shader_set_instance_ubo(
	VkDevice device,
	lise_shader* shader,
	void* data,
	lise_shader_instance* instance
)
{
	instance->ubo = data;

	// Set ubos to be dirty;
	memset(instance->ubo_dirty, true, shader->swapchain_image_count * sizeof(bool));
}

void lise_shader_set_instance_sampler(
	VkDevice device,
	lise_shader* shader,
	uint32_t sampler_index,
	lise_texture* sampler,
	lise_shader_instance* instance
)
{
	instance->samplers[sampler_index] = sampler;

	// Set samplers to be dirty.
	memset(
		instance->sampler_dirty + sampler_index * shader->swapchain_image_count,
		true,
		shader->swapchain_image_count * sizeof(bool)
	);
}

bool lise_shader_update_instance_ubo(
	VkDevice device,
	lise_shader* shader,
	uint32_t current_image,
	lise_shader_instance* instance
)
{
	// Uniform buffer objects.
	if (instance->ubo_dirty[current_image])
	{
		// Uniform is dirty. Update the instance uniform buffer. The descriptors do not need to be updated as they
		// point to the same address.
		lise_vulkan_buffer_load_data(
			device,
			&shader->instance_ub,
			(shader->swapchain_image_count * instance->id + current_image) * shader->instance_ubo_stride,
			shader->instance_ubo_size,
			0,
			instance->ubo
		);

		// Undirty ubo.
		instance->ubo_dirty[current_image] = false;
	}
	
	// Samplers.
	uint32_t d = 0; // Dirty sampler count.
	VkDescriptorImageInfo* instance_descriptor_image_infos =
		malloc(shader->instance_sampler_count * sizeof(VkDescriptorImageInfo));

	VkWriteDescriptorSet* instance_descriptor_writes =
		calloc(shader->instance_sampler_count, sizeof(VkWriteDescriptorSet));

	for (uint32_t i = 0; i < shader->instance_sampler_count; i++)
	{
		if (instance->sampler_dirty[i * shader->swapchain_image_count + current_image])
		{
			// Sampler is dirty. Update the descriptor to point to a new sampler.
			instance_descriptor_image_infos[d].sampler = instance->samplers[i]->sampler;
			instance_descriptor_image_infos[d].imageView = instance->samplers[i]->image.image_view;
			instance_descriptor_image_infos[d].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			instance_descriptor_writes[d].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			instance_descriptor_writes[d].dstSet = instance->descriptor_sets[current_image];
			instance_descriptor_writes[d].dstBinding = 1;
			instance_descriptor_writes[d].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			instance_descriptor_writes[d].descriptorCount = 1;
			instance_descriptor_writes[d].pImageInfo = &instance_descriptor_image_infos[d];

			d++;
		}
	}

	if (d)
	{
		vkUpdateDescriptorSets(device, d, instance_descriptor_writes, 0, NULL);
	}

	free(instance_descriptor_image_infos);
	free(instance_descriptor_writes);
}

// Static helper functions.
#define streq(x, y) strcmp(x, y) == 0

static lise_shader_uniform_type parse_uniform_type(const char* type)
{
	if (streq(type, "float"))
	{
		return LISE_SHADER_UNIFORM_TYPE_FLOAT32;
	}
	else if (streq(type, "vec2"))
	{
		return LISE_SHADER_UNIFORM_TYPE_FLOAT32_2;
	}
	else if (streq(type, "vec3"))
	{
		return LISE_SHADER_UNIFORM_TYPE_FLOAT32_3;
	}
	else if (streq(type, "vec4"))
	{
		return LISE_SHADER_UNIFORM_TYPE_FLOAT32_4;
	}
	else if (streq(type, "int8"))
	{
		return LISE_SHADER_UNIFORM_TYPE_INT8;
	}
	else if (streq(type, "uint8"))
	{
		return LISE_SHADER_UNIFORM_TYPE_UINT8;
	}
	else if (streq(type, "int16"))
	{
		return LISE_SHADER_UNIFORM_TYPE_INT16;
	}
	else if (streq(type, "uint16"))
	{
		return LISE_SHADER_UNIFORM_TYPE_UINT16;
	}
	else if (streq(type, "int32"))
	{
		return LISE_SHADER_UNIFORM_TYPE_INT32;
	}
	else if (streq(type, "uint32"))
	{
		return LISE_SHADER_UNIFORM_TYPE_UINT32;
	}
	else if (streq(type, "mat4"))
	{
		return LISE_SHADER_UNIFORM_TYPE_MATRIX_4;
	}
	else if (streq(type, "samp"))
	{
		return LISE_SHADER_UNIFORM_TYPE_SAMPLER;
	}
	else
	{
		return LISE_SHADER_UNIFORM_TYPE_CUSTOM;
	}
}

static uint64_t get_uniform_type_size(lise_shader_uniform_type type)
{
	switch (type)
	{
	case LISE_SHADER_UNIFORM_TYPE_INT8:
	case LISE_SHADER_UNIFORM_TYPE_UINT8:
		return 1;
	case LISE_SHADER_UNIFORM_TYPE_INT16:
	case LISE_SHADER_UNIFORM_TYPE_UINT16:
		return 2;
	case LISE_SHADER_UNIFORM_TYPE_FLOAT32:
	case LISE_SHADER_UNIFORM_TYPE_INT32:
	case LISE_SHADER_UNIFORM_TYPE_UINT32:
		return 4;
	case LISE_SHADER_UNIFORM_TYPE_FLOAT32_2:
		return 8;
	case LISE_SHADER_UNIFORM_TYPE_FLOAT32_3:
		return 12;
	case LISE_SHADER_UNIFORM_TYPE_FLOAT32_4:
		return 16;
	case LISE_SHADER_UNIFORM_TYPE_MATRIX_4:
		return 64;
	case LISE_SHADER_UNIFORM_TYPE_SAMPLER:
	default:
		return 0;
	}
}

static VkFormat parse_attribute_format(const char* type)
{
	if (streq(type, "float"))
	{
		return VK_FORMAT_R32_SFLOAT;
	}
	else if (streq(type, "vec2"))
	{
		return VK_FORMAT_R32G32_SFLOAT;
	}
	else if (streq(type, "vec3"))
	{
		return VK_FORMAT_R32G32B32_SFLOAT;
	}
	else if (streq(type, "vec4"))
	{
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	}
	else if (streq(type, "int8"))
	{
		return VK_FORMAT_R8_SINT;
	}
	else if (streq(type, "uint8"))
	{
		return VK_FORMAT_R8_UINT;
	}
	else if (streq(type, "int16"))
	{
		return VK_FORMAT_R16_SINT;
	}
	else if (streq(type, "uint16"))
	{
		return VK_FORMAT_R16_UINT;
	}
	else if (streq(type, "int32"))
	{
		return VK_FORMAT_R32_SINT;
	}
	else if (streq(type, "uint32"))
	{
		return VK_FORMAT_R32_UINT;
	}
	else
	{
		return 0;
	}
}

static uint64_t get_attribute_format_size(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_R8_SINT:
	case VK_FORMAT_R8_UINT:
		return 1;
	case VK_FORMAT_R16_SINT:
	case VK_FORMAT_R16_UINT:
		return 2;
	case VK_FORMAT_R32_SINT:
	case VK_FORMAT_R32_UINT:
	case VK_FORMAT_R32_SFLOAT:
		return 4;
	case VK_FORMAT_R32G32_SFLOAT:
		return 8;
	case VK_FORMAT_R32G32B32_SFLOAT:
		return 12;
	case VK_FORMAT_R32G32B32A32_SFLOAT:
		return 16;
	default:
		return 0;
	}
}
