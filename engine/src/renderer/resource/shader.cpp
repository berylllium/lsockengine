#include "renderer/resource/shader.hpp"

#include <vector>

#include <simple-logger.hpp>

#include "loader/shader_config_loader.hpp"
#include "math/vertex.hpp"
#include "renderer/resource/shader_stage.hpp"
#include "renderer/system/texture_system.hpp"

#define align(x, n) (((x - 1) | (n - 1)) + 1)

namespace lise
{

static ShaderUniformType parse_uniform_type(const std::string& type);
static uint64_t get_uniform_type_size(ShaderUniformType type);

static vk::Format parse_attribute_format(std::string type);
static uint64_t get_attribute_format_size(vk::Format format);

std::unique_ptr<Shader> Shader::create(
	const Device* device,
	const ShaderConfig& shader_config,
	const RenderPass* render_pass,
	uint32_t framebuffer_width,
	uint32_t framebuffer_height,
	uint32_t swapchain_image_count
)
{
	auto out = std::make_unique<Shader>();
	
	// Copy trivial data.
	out->device = device;
	out->name = shader_config.name;
	out->swapchain_image_count = swapchain_image_count;
	out->minimum_uniform_alignment = device->physical_device_properties.limits.minUniformBufferOffsetAlignment;

	// Create the shader stages.
	std::vector<std::unique_ptr<ShaderStage>> shader_stages;
	shader_stages.reserve(shader_config.stage_names.size());

	std::vector<vk::PipelineShaderStageCreateInfo> shader_stage_cis;
	shader_stage_cis.resize(shader_config.stage_names.size());

	for (uint64_t i = 0; i < shader_config.stage_names.size(); i++)
	{
		vk::ShaderStageFlagBits stage_flag;

		if (shader_config.stage_names[i] == "vertex")
		{
			stage_flag = vk::ShaderStageFlagBits::eVertex;
		}
		else if (shader_config.stage_names[i] == "fragment")
		{
			stage_flag = vk::ShaderStageFlagBits::eFragment;
		}
		else
		{
			sl::log_error(
				"Stage name provided in config `{}`, `{}` is not valid.",
				shader_config.name,
				shader_config.stage_names[i]
			);

			return nullptr;
		}

		auto stage = ShaderStage::create(device, shader_config.stage_file_names[i], stage_flag);

		if (!stage)
		{
			sl::log_error("Failed to open shader binary file for shader `{}`.", shader_config.name);

			return nullptr;
		}

		shader_stages.push_back(std::move(stage));

		shader_stage_cis[i] = shader_stages[i]->shader_stage_create_info;
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

	for (auto& u : shader_config.uniforms)
	{
		ShaderUniform uniform;

		uniform.type = parse_uniform_type(u.type);
		uniform.scope = static_cast<ShaderScope>(u.scope);
		uniform.name = u.name;

		uniform.size = get_uniform_type_size(uniform.type);

		switch (uniform.scope)
		{
		case ShaderScope::GLOBAL:
			uniform.offset = global_uniform_total_size;

			out->global_uniforms.push_back(uniform);

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
				out->instance_samplers.push_back(uniform);
				instance_has_sampler = true;
			}
			else
			{
				out->instance_uniforms.push_back(uniform);
				instance_has_uniform = true;
			}

			instance_uniform_total_size += uniform.size;
			break;
		case ShaderScope::LOCAL:
			uniform.offset = local_uniform_total_size;

			local_uniforms.push_back(uniform);

			local_uniform_total_size += uniform.size;
			break;
		}
	}

	out->instance_ubo_size = instance_uniform_total_size;
	out->global_ubo_size = global_uniform_total_size;

	out->instance_ubo_stride = align(out->instance_ubo_size, out->minimum_uniform_alignment);
	out->global_ubo_stride = align(out->global_ubo_size, out->minimum_uniform_alignment);

	// Create global descriptor set layout.
	std::vector<vk::DescriptorSetLayoutBinding> global_set_bindings;

	if (global_has_uniform)
	{
		vk::DescriptorSetLayoutBinding binding(
			global_set_bindings.size(),
			vk::DescriptorType::eUniformBuffer,
			1,
			vk::ShaderStageFlagBits::eVertex	// TODO: Make configurable.
		);

		global_set_bindings.push_back(binding);
	}

	// TODO: Handle global samplers.

	vk::DescriptorSetLayoutCreateInfo global_layout_ci(
		{},
		global_set_bindings
	);

	vk::Result r;

	std::tie(r, out->global_descriptor_set_layout) =
		out->device->logical_device.createDescriptorSetLayout(global_layout_ci);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to create global descriptor set layout.");
		return nullptr;
	}

	// Create global descriptor pool.
	vk::DescriptorPoolSize global_pool_size(vk::DescriptorType::eUniformBuffer, swapchain_image_count * swapchain_image_count);

	vk::DescriptorPoolCreateInfo global_pool_ci(
		{},
		swapchain_image_count,
		1,
		&global_pool_size
	);

	std::tie(r, out->global_descriptor_pool) = out->device->logical_device.createDescriptorPool(global_pool_ci);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to create global descriptor pool.");
		return nullptr;
	}

	// Create instance descriptor set layout.
	std::vector<vk::DescriptorSetLayoutBinding> instance_set_bindings;

	if (instance_has_uniform)
	{
		vk::DescriptorSetLayoutBinding binding(
			instance_set_bindings.size(),
			vk::DescriptorType::eUniformBuffer,
			1,
			vk::ShaderStageFlagBits::eFragment
		);

		instance_set_bindings.push_back(binding);
	}
	
	if (instance_has_sampler)
	{
		vk::DescriptorSetLayoutBinding binding(
			instance_set_bindings.size(),
			vk::DescriptorType::eCombinedImageSampler,
			1,
			vk::ShaderStageFlagBits::eFragment
		);

		instance_set_bindings.push_back(binding);
	}

	vk::DescriptorSetLayoutCreateInfo instance_layout_ci({}, instance_set_bindings);

	std::tie(r, out->instance_descriptor_set_layout) =
		out->device->logical_device.createDescriptorSetLayout(instance_layout_ci);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to create instance descriptor set layout.");
		return nullptr;
	}

	std::vector<vk::DescriptorPoolSize> instance_sizes(instance_set_bindings.size());

	for (uint64_t i = 0; i < instance_set_bindings.size(); i++)
	{
		instance_sizes[i].type = instance_set_bindings[i].descriptorType;
		instance_sizes[i].descriptorCount = LSHADER_MAX_INSTANCE_COUNT; // TODO: Add caluclation for exact dCount.
	}

	vk::DescriptorPoolCreateInfo instance_pool_ci(
		vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		LSHADER_MAX_INSTANCE_COUNT,
		instance_sizes
	);

	std::tie(r, out->instance_descriptor_pool) = out->device->logical_device.createDescriptorPool(instance_pool_ci);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to create instance descriptor pool.");
		return nullptr;
	}

	// Push constants / Local uniforms.
	uint32_t push_constant_count = local_uniforms.size();
	std::vector<vk::PushConstantRange> push_constant_ranges(push_constant_count);

	for (uint32_t i = 0; i < local_uniforms.size(); i++)
	{
		push_constant_ranges[i].stageFlags = vk::ShaderStageFlagBits::eVertex; // TODO: Make configurable.
		push_constant_ranges[i].size = local_uniforms[i].size;
		push_constant_ranges[i].offset = local_uniforms[i].offset;
	}

	// Pipeline creation
	vk::Viewport viewport;
	viewport.x = 0.0f;
	viewport.y = (float) framebuffer_height;
	viewport.width = (float) framebuffer_width;
	viewport.height = -(float) framebuffer_height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Scissor
	vk::Rect2D scissor;
	scissor.offset.x = scissor.offset.y = 0;
	scissor.extent.width = framebuffer_width;
	scissor.extent.height = framebuffer_height;

	// Vertex attributes
	std::vector<vk::VertexInputAttributeDescription> attribs(shader_config.attributes.size());
	
	out->vertex_attributes.resize(shader_config.attributes.size());
	
	uint32_t offset = 0;
	for (uint32_t i = 0; i < shader_config.attributes.size(); i++)
	{
		attribs[i].binding = 0;
		attribs[i].location = i;
		attribs[i].format = parse_attribute_format(shader_config.attributes[i].type);
		attribs[i].offset = offset;

		uint32_t attrib_size = get_attribute_format_size(attribs[i].format);

		offset += attrib_size;

		out->vertex_attributes[i].name = shader_config.attributes[i].name;
		out->vertex_attributes[i].type = attribs[i].format;
		out->vertex_attributes[i].size = attrib_size;
	}

	// Create pipeline.
	std::vector<vk::DescriptorSetLayout> set_layouts = {
		out->global_descriptor_set_layout,
		out->instance_descriptor_set_layout
	};

	out->pipeline = Pipeline::create(
		device,
		render_pass,
		sizeof(vertex),
		attribs,
		set_layouts,
		shader_stage_cis,
		push_constant_ranges,
		viewport,
		scissor,
		false,
		true
	);
	
	if (!out->pipeline)
	{
		sl::log_error("Failed to create graphics pipeline.");
		return nullptr;
	}

	// Allocate the global uniform buffer object.
	out->global_ub = VulkanBuffer::create(
		device,
		out->global_ubo_stride * swapchain_image_count, // We use the stride, not the total size.
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible |
		vk::MemoryPropertyFlagBits::eHostCoherent,
		true
	);

	if (!out->global_ub)
	{
		sl::log_error("Failed to allocate the global uniform buffer object.");
		return nullptr;
	}

	// Allocate global descriptor sets.
	std::vector<vk::DescriptorSetLayout> global_set_layouts(swapchain_image_count, out->global_descriptor_set_layout);

	vk::DescriptorSetAllocateInfo global_set_allocate_info(
		out->global_descriptor_pool,
		global_set_layouts
	);

	std::tie(r, out->global_descriptor_sets) = device->logical_device.allocateDescriptorSets(global_set_allocate_info);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to allocate global descriptor sets.");
		return nullptr;
	}

	// Setup the descriptors to point to the corrosponding location in the global uniform buffer.
	std::vector<vk::DescriptorBufferInfo> global_descriptor_buffer_infos(swapchain_image_count);

	std::vector<vk::WriteDescriptorSet> global_descriptor_writes(swapchain_image_count);

	for (uint32_t i = 0; i < swapchain_image_count; i++)
	{
		global_descriptor_buffer_infos[i].buffer = out->global_ub->handle;
		global_descriptor_buffer_infos[i].offset = i * out->global_ubo_stride;
		global_descriptor_buffer_infos[i].range = global_uniform_total_size;

		global_descriptor_writes[i].dstSet = out->global_descriptor_sets[i];
		global_descriptor_writes[i].dstBinding = 0;
		global_descriptor_writes[i].descriptorType = vk::DescriptorType::eUniformBuffer;
		global_descriptor_writes[i].descriptorCount = 1;
		global_descriptor_writes[i].pBufferInfo = &global_descriptor_buffer_infos[i];
	}

	device->logical_device.updateDescriptorSets(global_descriptor_writes, nullptr);

	// Set global ubos to be dirty.
	out->global_ubo_dirty.resize(swapchain_image_count, true);

	// Allocate the instance uniform buffer.
	out->instance_ub = VulkanBuffer::create(
		device,
		instance_uniform_total_size * LSHADER_MAX_INSTANCE_COUNT * swapchain_image_count,
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible |
		vk::MemoryPropertyFlagBits::eHostCoherent,
		true
	);

	if (!out->instance_ub)
	{
		sl::log_error("Failed to create the object unbiform buffer.");
		return nullptr;
	}

	// Allocate the instance uniform buffer object free list.
	out->instance_ubo_free_list.resize(LSHADER_MAX_INSTANCE_COUNT, true);

	return out;
}

Shader::~Shader()
{
	instances.clear();

	// Destroy the descriptor pools.
	device->logical_device.destroy(global_descriptor_pool);
	device->logical_device.destroy(instance_descriptor_pool);

	// Destroy the descriptor set layouts.
	device->logical_device.destroy(global_descriptor_set_layout);
	device->logical_device.destroy(instance_descriptor_set_layout);
}

void Shader::use(CommandBuffer* command_buffer, uint32_t current_image)
{
	command_buffer->handle.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		pipeline->pipeline_layout,
		0,
		1,
		&global_descriptor_sets[current_image],
		0,
		nullptr
	);

	pipeline->bind(command_buffer, vk::PipelineBindPoint::eGraphics);
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
			{},
			global_ubo
		);

		// Undirty ubo.
		global_ubo_dirty[current_image] = false;
	}
}

Shader::Instance* Shader::allocate_instance()
{
	auto out = std::make_unique<Shader::Instance>();

	out->shader = this;

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
		sl::log_error("Failed to find a free slot in the instance uniform buffer of shader `{}`.", name);
		return nullptr;
	}

	// Claim slot.
	instance_ubo_free_list[id] = false;
	out->id = id;

	// Allocate arrays.
	out->ubo_dirty.resize(swapchain_image_count, true);

	if (instance_samplers.size() > 0)
	{
		out->samplers.resize(instance_samplers.size(), texture_system_get_default_texture());

		out->sampler_dirty.resize(swapchain_image_count * instance_samplers.size(), true);
	}

	// Allocate the descriptor sets.
	std::vector<vk::DescriptorSetLayout> layouts(swapchain_image_count, instance_descriptor_set_layout);

	vk::DescriptorSetAllocateInfo d_set_ai(
		instance_descriptor_pool,
		layouts
	);

	vk::Result r;

	std::tie(r, out->descriptor_sets) = device->logical_device.allocateDescriptorSets(d_set_ai);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to allocate instance descriptor sets for shader `{}`.", name);
		return nullptr;
	}

	// Point the descriptors to the corrosponding location in the uniform buffer.
	std::vector<vk::DescriptorBufferInfo> instance_descriptor_buffer_infos(swapchain_image_count);
	std::vector<vk::WriteDescriptorSet> instance_descriptor_writes(swapchain_image_count);

	for (uint32_t i = 0; i < swapchain_image_count; i++)
	{
		instance_descriptor_buffer_infos[i].buffer = instance_ub->handle;
		instance_descriptor_buffer_infos[i].offset = (swapchain_image_count * id + i) * instance_ubo_stride;
		instance_descriptor_buffer_infos[i].range = instance_ubo_size;

		instance_descriptor_writes[i].dstSet = out->descriptor_sets[i];
		instance_descriptor_writes[i].dstBinding = 0;
		instance_descriptor_writes[i].descriptorType = vk::DescriptorType::eUniformBuffer;
		instance_descriptor_writes[i].descriptorCount = 1;
		instance_descriptor_writes[i].pBufferInfo = &instance_descriptor_buffer_infos[i];
	}

	device->logical_device.updateDescriptorSets(instance_descriptor_writes, nullptr);

	return (*(instances.insert({id, std::move(out)}).first)).second.get();
}

void Shader::deallocate_instance(uint64_t id)
{
	// Return slot to the free list.
	instance_ubo_free_list[id] = true;

	instances.erase(id);
}

Shader::Instance::~Instance()
{
	if (shader)
	{
		shader->device->logical_device.free(shader->instance_descriptor_pool, descriptor_sets);
	}
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
			{},
			ubo
		);

		// Undirty ubo.
		ubo_dirty[current_image] = false;
	}
	
	// Samplers.
	uint32_t d = 0; // Dirty sampler count.
	std::vector<vk::DescriptorImageInfo> instance_descriptor_image_infos(shader->instance_samplers.size());
	std::vector<vk::WriteDescriptorSet> instance_descriptor_writes(shader->instance_samplers.size());

	for (uint32_t i = 0; i < shader->instance_samplers.size(); i++)
	{
		if (sampler_dirty[i * shader->swapchain_image_count + current_image])
		{
			// Sampler is dirty. Update the descriptor to point to a new sampler.
			instance_descriptor_image_infos[d].sampler = samplers[i]->sampler;
			instance_descriptor_image_infos[d].imageView = samplers[i]->image->image_view;
			instance_descriptor_image_infos[d].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

			instance_descriptor_writes[d].dstSet = descriptor_sets[current_image];
			instance_descriptor_writes[d].dstBinding = 1;
			instance_descriptor_writes[d].descriptorType = vk::DescriptorType::eCombinedImageSampler;
			instance_descriptor_writes[d].descriptorCount = 1;
			instance_descriptor_writes[d].pImageInfo = &instance_descriptor_image_infos[d];

			d++;
		}
	}

	if (d)
	{
		shader->device->logical_device.updateDescriptorSets(instance_descriptor_writes, nullptr);
	}
}

void Shader::Instance::bind_descriptor_set(CommandBuffer* command_buffer, uint32_t current_image)
{
	command_buffer->handle.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		shader->pipeline->pipeline_layout,
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

static vk::Format parse_attribute_format(std::string type)
{
	if (type == "float")
	{
		return vk::Format::eR32Sfloat;
	}
	else if (type == "vec2")
	{
		return vk::Format::eR32G32Sfloat;
	}
	else if (type == "vec3")
	{
		return vk::Format::eR32G32B32Sfloat;
	}
	else if (type == "vec4")
	{
		return vk::Format::eR32G32B32A32Sfloat;
	}
	else if (type == "int8")
	{
		return vk::Format::eR8Sint;
	}
	else if (type == "uint8")
	{
		return vk::Format::eR8Uint;
	}
	else if (type == "int16")
	{
		return vk::Format::eR16Sint;
	}
	else if (type == "uint16")
	{
		return vk::Format::eR16Uint;
	}
	else if (type == "int32")
	{
		return vk::Format::eR32Sint;
	}
	else if (type == "uint32")
	{
		return vk::Format::eR32Uint;
	}
	else
	{
		return vk::Format::eR32Sint;
	}
}

static uint64_t get_attribute_format_size(vk::Format format)
{
	switch (format)
	{
	case vk::Format::eR8Sint:
	case vk::Format::eR8Uint:
		return 1;
	case vk::Format::eR16Sint:
	case vk::Format::eR16Uint:
		return 2;
	case vk::Format::eR32Sint:
	case vk::Format::eR32Uint:
	case vk::Format::eR32Sfloat:
		return 4;
	case vk::Format::eR32G32Sfloat:
		return 8;
	case vk::Format::eR32G32B32Sfloat:
		return 12;
	case vk::Format::eR32G32B32A32Sfloat:
		return 16;
	default:
		return 0;
	}
}

}
