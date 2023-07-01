#include "renderer/resource/shader.hpp"

#include <vector>

#include "core/logger.hpp"
#include "loader/shader_config_loader.hpp"
#include "renderer/resource/shader_stage.hpp"
#include "renderer/system/texture_system.hpp"

#define align(x, n) (((x - 1) | (n - 1)) + 1)

namespace lise
{

static ShaderUniformType parse_uniform_type(const std::string& type);
static uint64_t get_uniform_type_size(ShaderUniformType type);

static VkFormat parse_attribute_format(std::string type);
static uint64_t get_attribute_format_size(VkFormat format);

Shader::Shader(
	const Device& device,
	const ShaderConfig& shader_config,
	const RenderPass& render_pass, // TODO: Temporary, add a render pass system.
	uint32_t framebuffer_width,
	uint32_t framebuffer_height,
	uint32_t swapchain_image_count
) : device(device), name(shader_config.name), swapchain_image_count(swapchain_image_count),
	minimum_uniform_alignment(device.get_properties().limits.minUniformBufferOffsetAlignment)
{
	// Create the shader stages.
	std::vector<ShaderStage> shader_stages;
	shader_stages.reserve(shader_config.stage_names.size());

	std::vector<VkPipelineShaderStageCreateInfo> shader_stage_cis;
	shader_stage_cis.resize(shader_config.stage_names.size());

	for (uint64_t i = 0; i < shader_config.stage_names.size(); i++)
	{
		VkShaderStageFlagBits stage_flag;

		if (shader_config.stage_names[i] == "vertex")
		{
			stage_flag = VK_SHADER_STAGE_VERTEX_BIT;
		}
		else if (shader_config.stage_names[i] == "fragment")
		{
			stage_flag = VK_SHADER_STAGE_FRAGMENT_BIT;
		}
		else
		{
			LERROR(
				"Stage name provided in config `%s`, `%s` is not valid.",
				shader_config.name,
				shader_config.stage_names[i]
			);

			throw std::exception();
		}
		
		try
		{
			//std::string file_name = shader_config.stage_file_names[i];
			shader_stages.emplace_back(device, shader_config.stage_file_names[i], stage_flag);
		}
		catch(std::exception e)
		{
			LERROR("Failed to open shader binary file for config file `%s`.", shader_config.name);

			throw std::exception();
		}

		shader_stage_cis[i] = shader_stages[i].get_pipeline_shader_stage_creation_info();
	}

	// Sort the uniforms.
	bool global_has_uniform = false;
	bool global_has_sampler = false;
	uint64_t global_uniform_total_size = 0;

	bool instance_has_uniform = false;
	bool instance_has_sampler = false;
	uint64_t instance_uniform_total_size = 0;

	uint64_t local_uniform_total_size = 0;
	std::vector<ShaderUniform> local_uniforms;

	for (uint64_t i = 0; i < shader_config.uniforms.size(); i++)
	{
		ShaderUniform uniform;

		uniform.type = parse_uniform_type(shader_config.uniforms[i].type);
		uniform.scope = static_cast<ShaderScope>(shader_config.uniforms[i].scope);
		uniform.name = shader_config.uniforms[i].name;

		uniform.size = get_uniform_type_size(uniform.type);


		switch (uniform.scope)
		{
		case ShaderScope::GLOBAL:
			uniform.offset = global_uniform_total_size;

			global_uniforms.push_back(std::move(uniform));

			if (uniform.type == ShaderUniformType::SAMPLER)
			{
				global_has_sampler = true;
			}
			else
			{
				global_has_uniform = true;
			}

			global_uniform_total_size += uniform.size;
			break;
		case ShaderScope::INSTANCE:
			uniform.offset = instance_uniform_total_size;

			if (uniform.type == ShaderUniformType::SAMPLER)
			{
				instance_samplers.push_back(std::move(uniform));
				instance_has_sampler = true;
			}
			else
			{
				instance_uniforms.push_back(std::move(uniform));
				instance_has_uniform = true;
			}

			instance_uniform_total_size += uniform.size;
			break;
		case ShaderScope::LOCAL:
			uniform.offset = local_uniform_total_size;

			local_uniforms.push_back(std::move(uniform));

			local_uniform_total_size += uniform.size;
			break;
		}
	}

	instance_ubo_size = instance_uniform_total_size;
	global_ubo_size = global_uniform_total_size;

	instance_ubo_stride = align(instance_ubo_size, minimum_uniform_alignment);
	global_ubo_stride = align(global_ubo_size, minimum_uniform_alignment);

	// Create global descriptor set layout.
	std::vector<VkDescriptorSetLayoutBinding> global_set_bindings;

	if (global_has_uniform)
	{
		VkDescriptorSetLayoutBinding binding = {};
		binding.binding = global_set_bindings.size();
		binding.descriptorCount = 1;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // TODO: Make configurable.

		global_set_bindings.push_back(binding);
	}

	// TODO: Handle global samplers.

	VkDescriptorSetLayoutCreateInfo global_layout_ci = {};
	global_layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	global_layout_ci.bindingCount = global_set_bindings.size();
	global_layout_ci.pBindings = global_set_bindings.data();

	if (vkCreateDescriptorSetLayout(device, & global_layout_ci, NULL, &global_descriptor_set_layout) != VK_SUCCESS)
	{
		LERROR("Failed to create global descriptor set layout.");

		throw std::exception();
	}

	// Create global descriptor pool.
	VkDescriptorPoolSize global_pool_size;
	global_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	global_pool_size.descriptorCount = swapchain_image_count * swapchain_image_count;

	VkDescriptorPoolCreateInfo global_pool_ci = {};
	global_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	global_pool_ci.poolSizeCount = 1;
	global_pool_ci.pPoolSizes = &global_pool_size;
	global_pool_ci.maxSets = swapchain_image_count;

	if (vkCreateDescriptorPool(device, &global_pool_ci, NULL, &global_descriptor_pool) != VK_SUCCESS)
	{
		LERROR("Failed to create global descriptor pool.");
		
		throw std::exception();
	}

	// Create instance descriptor set layout.
	std::vector<VkDescriptorSetLayoutBinding> instance_set_bindings;

	if (instance_has_uniform)
	{
		VkDescriptorSetLayoutBinding binding = {};
		binding.binding = instance_set_bindings.size();
		binding.descriptorCount = 1;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // TODO: Make configurable.

		instance_set_bindings.push_back(binding);
	}
	
	if (instance_has_sampler)
	{
		VkDescriptorSetLayoutBinding binding = {};
		binding.binding = instance_set_bindings.size();
		binding.descriptorCount = 1;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // TODO: Make configurable.

		instance_set_bindings.push_back(binding);
	}

	VkDescriptorSetLayoutCreateInfo instance_layout_ci = {};
	instance_layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	instance_layout_ci.bindingCount = instance_set_bindings.size();
	instance_layout_ci.pBindings = instance_set_bindings.data();

	if (vkCreateDescriptorSetLayout(device, &instance_layout_ci, NULL, &instance_descriptor_set_layout) != VK_SUCCESS)
	{
		LERROR("Failed to create instance descriptor set layout.");

		throw std::exception();
	}

	std::unique_ptr<VkDescriptorPoolSize[]> instance_sizes =
		std::make_unique<VkDescriptorPoolSize[]>(instance_set_bindings.size());

	for (uint64_t i = 0; i < instance_set_bindings.size(); i++)
	{
		instance_sizes[i].type = instance_set_bindings[i].descriptorType;
		instance_sizes[i].descriptorCount = LSHADER_MAX_INSTANCE_COUNT; // TODO: Add caluclation for exact dCount.
	}

	VkDescriptorPoolCreateInfo instance_pool_ci = {};
	instance_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	instance_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	instance_pool_ci.poolSizeCount = instance_set_bindings.size();
	instance_pool_ci.pPoolSizes = instance_sizes.get();
	instance_pool_ci.maxSets = LSHADER_MAX_INSTANCE_COUNT;

	if (vkCreateDescriptorPool(device, &instance_pool_ci, NULL, &instance_descriptor_pool) != VK_SUCCESS)
	{
		LERROR("Failed to create instance descriptor pool.");

		throw std::exception();
	}

	// Push constants / Local uniforms.
	uint32_t push_constant_count = local_uniforms.size();
	std::vector<VkPushConstantRange> push_constant_ranges;
	push_constant_ranges.resize(local_uniforms.size());

	for (uint32_t i = 0; i < local_uniforms.size(); i++)
	{
		push_constant_ranges[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // TODO: Make configurable.
		push_constant_ranges[i].size = local_uniforms[i].size;
		push_constant_ranges[i].offset = local_uniforms[i].offset;
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

	// Vertex attributes
	std::vector<VkVertexInputAttributeDescription> attribs;
	attribs.resize(shader_config.attributes.size());
	
	vertex_attributes.resize(shader_config.attributes.size());
	
	uint32_t offset = 0;
	for (uint32_t i = 0; i < shader_config.attributes.size(); i++)
	{
		attribs[i].binding = 0;
		attribs[i].location = i;
		attribs[i].format = parse_attribute_format(shader_config.attributes[i].type);
		attribs[i].offset = offset;

		uint32_t attrib_size = get_attribute_format_size(attribs[i].format);

		offset += attrib_size;

		vertex_attributes[i].name = shader_config.attributes[i].name;
		vertex_attributes[i].type = attribs[i].format;
		vertex_attributes[i].size = attrib_size;
	}

	// Create pipeline.
	std::vector<VkDescriptorSetLayout> set_layouts = {
		global_descriptor_set_layout,
		instance_descriptor_set_layout
	};

	try
	{
		pipeline = new Pipeline(
			device,
			render_pass,
			attribs,
			set_layouts,
			shader_stage_cis,
			push_constant_ranges,
			viewport,
			scissor,
			false
		);
	}
	catch (std::exception e)
	{
		LERROR("Failed to create graphics pipeline.");

		throw std::exception();
	}

	// Allocate the global uniform buffer object.
	try
	{
		global_ub = new VulkanBuffer(
			device,
			device.get_memory_properties(),
			global_ubo_stride * swapchain_image_count, // We use the stride, not the total size.
			static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			true
		);
	}
	catch (std::exception e)
	{
		LERROR("Failed to allocate the global uniform buffer object.");

		throw std::exception();
	}

	// Allocate global descriptor sets.
	std::vector<VkDescriptorSetLayout> global_set_layouts(swapchain_image_count);

	for (uint32_t i = 0; i < swapchain_image_count; i++)
	{
		global_set_layouts[i] = global_descriptor_set_layout;
	}

	VkDescriptorSetAllocateInfo global_set_allocate_info = {};
	global_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	global_set_allocate_info.descriptorPool = global_descriptor_pool;
	global_set_allocate_info.descriptorSetCount = swapchain_image_count;
	global_set_allocate_info.pSetLayouts = global_set_layouts.data();

	global_descriptor_sets.reserve(swapchain_image_count);
	if (vkAllocateDescriptorSets(device, &global_set_allocate_info, global_descriptor_sets.data()) != VK_SUCCESS)
	{
		LERROR("Failed to allocate global descriptor sets.");

		throw std::exception();
	}

	// Setup the descriptors to point to the corrosponding location in the global uniform buffer.
	std::vector<VkDescriptorBufferInfo> global_descriptor_buffer_infos(swapchain_image_count);

	std::vector<VkWriteDescriptorSet> global_descriptor_writes(swapchain_image_count);

	for (uint32_t i = 0; i < swapchain_image_count; i++)
	{
		global_descriptor_buffer_infos[i].buffer = *global_ub;
		global_descriptor_buffer_infos[i].offset = i * global_ubo_stride;
		global_descriptor_buffer_infos[i].range = global_uniform_total_size;

		global_descriptor_writes[i] = {};
		global_descriptor_writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		global_descriptor_writes[i].dstSet = global_descriptor_sets[i];
		global_descriptor_writes[i].dstBinding = 0;
		global_descriptor_writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		global_descriptor_writes[i].descriptorCount = 1;
		global_descriptor_writes[i].pBufferInfo = &global_descriptor_buffer_infos[i];
	}

	vkUpdateDescriptorSets(device, swapchain_image_count, global_descriptor_writes.data(), 0, NULL);

	// Set global ubos to be dirty.
	global_ubo_dirty.resize(swapchain_image_count, true);

	// Allocate the instance uniform buffer.
	try
	{
		instance_ub = new VulkanBuffer(
			device,
			device.get_memory_properties(),
			instance_uniform_total_size * LSHADER_MAX_INSTANCE_COUNT * swapchain_image_count,
			static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			true
		);
	}
	catch (std::exception e)
	{
		LERROR("Failed to create the object unbiform buffer.");

		throw std::exception();
	}

	// Allocate the instance uniform buffer object free list.
	instance_ubo_free_list.resize(LSHADER_MAX_INSTANCE_COUNT, true);
}

Shader::~Shader()
{
	instances.clear();

	// Free the pipeline.
	delete pipeline;
	
	delete global_ub;

	delete instance_ub;

	// Destroy the descriptor pools.
	vkDestroyDescriptorPool(device, global_descriptor_pool, NULL);
	vkDestroyDescriptorPool(device, instance_descriptor_pool, NULL);

	// Destroy the descriptor set layouts.
	vkDestroyDescriptorSetLayout(device, global_descriptor_set_layout, NULL);
	vkDestroyDescriptorSetLayout(device, instance_descriptor_set_layout, NULL);
}

void Shader::use(CommandBuffer& command_buffer, uint32_t current_image)
{
	vkCmdBindDescriptorSets(
		command_buffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline->get_layout(),
		0,
		1,
		&global_descriptor_sets[current_image],
		0,
		NULL
	);

	pipeline->bind(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void Shader::set_global_ubo(void* data)
{
	global_ubo = data;
	
	// Set ubos to be dirty.
	for (size_t i = 0; i < swapchain_image_count; i++)
	{
		global_ubo_dirty[i] = true;
	}
}

void Shader::update_global_uniforms(uint32_t current_image)
{
	// Uniform buffers.
	if (global_ubo_dirty[current_image])
	{
		global_ub->load_data(
			current_image * global_ubo_stride,
			global_ubo_size,
			0,
			global_ubo
		);

		// Undirty ubo.
		global_ubo_dirty[current_image] = false;
	}
}

const Pipeline& Shader::get_pipeline() const
{
	return *pipeline;
}

Shader::Instance* Shader::allocate_instance()
{
	Instance out_instance;

	out_instance.shader = this;

	// Iterate the free list and pick a free spot.
	bool slot_found = false;
	uint64_t id;

	for (uint64_t i = 0; i < LSHADER_MAX_INSTANCE_COUNT; i++)
	{
		if (instance_ubo_free_list[i])
		{
			id = i;
			slot_found = true;
			break;
		}
	}

	if (!slot_found)
	{
		LERROR("Failed to find a free slot in the instance uniform buffer of shader `%s`.", name);

		return nullptr;
	}

	// Claim slot.
	instance_ubo_free_list[id] = false;
	out_instance.id = id;

	// Allocate arrays.
	out_instance.descriptor_sets.resize(swapchain_image_count);

	out_instance.ubo_dirty.resize(swapchain_image_count, true);

	if (instance_samplers.size() > 0)
	{
		out_instance.samplers.resize(instance_samplers.size(), texture_system_get_default_texture());

		out_instance.sampler_dirty.resize(swapchain_image_count * instance_samplers.size(), true);
	}

	// Allocate the descriptor sets.
	std::vector<VkDescriptorSetLayout> layouts(swapchain_image_count, instance_descriptor_set_layout);

	VkDescriptorSetAllocateInfo d_set_ai = {};
	d_set_ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	d_set_ai.descriptorPool = instance_descriptor_pool;
	d_set_ai.descriptorSetCount = swapchain_image_count;
	d_set_ai.pSetLayouts = layouts.data();

	if (vkAllocateDescriptorSets(device, &d_set_ai, out_instance.descriptor_sets.data()) != VK_SUCCESS)
	{
		LERROR("Failed to allocate instance descriptor sets for shader `%s`.", name);

		return nullptr;
	}

	// Point the descriptors to the corrosponding location in the uniform buffer.
	std::vector<VkDescriptorBufferInfo> instance_descriptor_buffer_infos(swapchain_image_count);
	std::vector<VkWriteDescriptorSet> instance_descriptor_writes(swapchain_image_count);

	for (uint32_t i = 0; i < swapchain_image_count; i++)
	{
		instance_descriptor_buffer_infos[i].buffer = *instance_ub;
		instance_descriptor_buffer_infos[i].offset = (swapchain_image_count * id + i) * instance_ubo_stride;
		instance_descriptor_buffer_infos[i].range = instance_ubo_size;

		instance_descriptor_writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		instance_descriptor_writes[i].dstSet = out_instance.descriptor_sets[i];
		instance_descriptor_writes[i].dstBinding = 0;
		instance_descriptor_writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		instance_descriptor_writes[i].descriptorCount = 1;
		instance_descriptor_writes[i].pBufferInfo = &instance_descriptor_buffer_infos[i];
	}

	vkUpdateDescriptorSets(device, swapchain_image_count, instance_descriptor_writes.data(), 0, NULL);

	return &(*(instances.insert({id, std::move(out_instance)}).first)).second;
}

void Shader::deallocate_instance(uint64_t id)
{
	// Return slot to the free list.
	instance_ubo_free_list[id] = true;

	instances.erase(id);
}

Shader::Instance::Instance(Shader::Instance&& other)
{
	id = other.id;
	other.id = 0;

	descriptor_sets = other.descriptor_sets;
	other.descriptor_sets.clear();

	ubo = other.ubo;
	other.ubo = nullptr;
	
	ubo_dirty = other.ubo_dirty;
	other.ubo_dirty.clear();

	samplers = other.samplers;
	other.samplers.clear();

	sampler_dirty = other.sampler_dirty;
	other.sampler_dirty.clear();

	shader = other.shader;
	other.shader = nullptr;
}

Shader::Instance::~Instance()
{
	if (shader)
	{
		vkFreeDescriptorSets(
			shader->device,
			shader->instance_descriptor_pool,
			shader->swapchain_image_count,
			descriptor_sets.data()
		);
	}
}

Shader::Instance& Shader::Instance::operator = (Shader::Instance&& other)
{
	this->~Instance();

	id = other.id;
	other.id = 0;

	descriptor_sets = other.descriptor_sets;
	other.descriptor_sets.clear();

	ubo = other.ubo;
	other.ubo = nullptr;
	
	ubo_dirty = other.ubo_dirty;
	other.ubo_dirty.clear();

	samplers = other.samplers;
	other.samplers.clear();

	sampler_dirty = other.sampler_dirty;
	other.sampler_dirty.clear();

	shader = other.shader;
	other.shader = nullptr;

	return *this;
}

void Shader::Instance::set_ubo(void* data)
{
	ubo = data;

	// Set ubos to be dirty;
	for (uint64_t i = 0; i < ubo_dirty.size(); i++)
	{
		ubo_dirty[i] = true;
	}
}

void Shader::Instance::set_sampler(uint32_t sampler_index, const Texture* sampler)
{
	samplers[sampler_index] = sampler;

	// Set samplers to be dirty.
	for (uint64_t i = 0; i < sampler_dirty.size(); i++)
	{
		sampler_dirty[i] = true;
	}
}

void Shader::Instance::update_ubo(uint32_t current_image)
{
	// Uniform buffer objects.
	if (ubo_dirty[current_image])
	{
		// Uniform is dirty. Update the instance uniform buffer. The descriptors do not need to be updated as they
		// point to the same address.
		shader->instance_ub->load_data(
			(shader->swapchain_image_count * id + current_image) * shader->instance_ubo_stride,
			shader->instance_ubo_size,
			0,
			ubo
		);

		// Undirty ubo.
		ubo_dirty[current_image] = false;
	}
	
	// Samplers.
	uint32_t d = 0; // Dirty sampler count.
	std::vector<VkDescriptorImageInfo> instance_descriptor_image_infos(shader->instance_samplers.size());
	std::vector<VkWriteDescriptorSet> instance_descriptor_writes(shader->instance_samplers.size());

	for (uint32_t i = 0; i < shader->instance_samplers.size(); i++)
	{
		if (sampler_dirty[i * shader->swapchain_image_count + current_image])
		{
			// Sampler is dirty. Update the descriptor to point to a new sampler.
			instance_descriptor_image_infos[d].sampler = samplers[i]->get_sampler();
			instance_descriptor_image_infos[d].imageView = samplers[i]->get_image_view();
			instance_descriptor_image_infos[d].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			instance_descriptor_writes[d].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			instance_descriptor_writes[d].dstSet = descriptor_sets[current_image];
			instance_descriptor_writes[d].dstBinding = 1;
			instance_descriptor_writes[d].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			instance_descriptor_writes[d].descriptorCount = 1;
			instance_descriptor_writes[d].pImageInfo = &instance_descriptor_image_infos[d];

			d++;
		}
	}

	if (d)
	{
		vkUpdateDescriptorSets(shader->device, d, instance_descriptor_writes.data(), 0, NULL);
	}
}

void Shader::Instance::bind_descriptor_set(CommandBuffer& command_buffer, uint32_t current_image)
{
	vkCmdBindDescriptorSets(
		command_buffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		shader->get_pipeline().get_layout(),
		1,
		1,
		&descriptor_sets[current_image],
		0,
		0
	);
}

// Static helper functions.
static ShaderUniformType parse_uniform_type(const std::string& type)
{
	if (type == "float")
	{
		return ShaderUniformType::FLOAT32;
	}
	else if (type == "vec2")
	{
		return ShaderUniformType::FLOAT32_2;
	}
	else if (type == "vec3")
	{
		return ShaderUniformType::FLOAT32_3;
	}
	else if (type == "vec4")
	{
		return ShaderUniformType::FLOAT32_4;
	}
	else if (type == "int8")
	{
		return ShaderUniformType::INT8;
	}
	else if (type == "uint8")
	{
		return ShaderUniformType::UINT8;
	}
	else if (type == "int16")
	{
		return ShaderUniformType::INT16;
	}
	else if (type == "uint16")
	{
		return ShaderUniformType::UINT16;
	}
	else if (type == "int32")
	{
		return ShaderUniformType::INT32;
	}
	else if (type == "uint32")
	{
		return ShaderUniformType::UINT32;
	}
	else if (type == "mat4")
	{
		return ShaderUniformType::MATRIX_4;
	}
	else if (type == "samp")
	{
		return ShaderUniformType::SAMPLER;
	}
	else
	{
		return ShaderUniformType::CUSTOM;
	}
}

static uint64_t get_uniform_type_size(ShaderUniformType type)
{
	switch (type)
	{
	case ShaderUniformType::INT8:
	case ShaderUniformType::UINT8:
		return 1;
	case ShaderUniformType::INT16:
	case ShaderUniformType::UINT16:
		return 2;
	case ShaderUniformType::FLOAT32:
	case ShaderUniformType::INT32:
	case ShaderUniformType::UINT32:
		return 4;
	case ShaderUniformType::FLOAT32_2:
		return 8;
	case ShaderUniformType::FLOAT32_3:
		return 12;
	case ShaderUniformType::FLOAT32_4:
		return 16;
	case ShaderUniformType::MATRIX_4:
		return 64;
	case ShaderUniformType::SAMPLER:
	default:
		return 0;
	}
}

static VkFormat parse_attribute_format(std::string type)
{
	if (type == "float")
	{
		return VK_FORMAT_R32_SFLOAT;
	}
	else if (type == "vec2")
	{
		return VK_FORMAT_R32G32_SFLOAT;
	}
	else if (type == "vec3")
	{
		return VK_FORMAT_R32G32B32_SFLOAT;
	}
	else if (type == "vec4")
	{
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	}
	else if (type == "int8")
	{
		return VK_FORMAT_R8_SINT;
	}
	else if (type == "uint8")
	{
		return VK_FORMAT_R8_UINT;
	}
	else if (type == "int16")
	{
		return VK_FORMAT_R16_SINT;
	}
	else if (type == "uint16")
	{
		return VK_FORMAT_R16_UINT;
	}
	else if (type == "int32")
	{
		return VK_FORMAT_R32_SINT;
	}
	else if (type == "uint32")
	{
		return VK_FORMAT_R32_UINT;
	}
	else
	{
		return VK_FORMAT_R32_SINT;
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

}
