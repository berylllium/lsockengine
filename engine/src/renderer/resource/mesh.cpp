#include "renderer/resource/mesh.hpp"

#include <format>

#include <simple-logger.hpp>

namespace lise
{

static void upload_data_range(
	const Device* device,
	vk::CommandPool command_pool,
	vk::Queue queue,
	VulkanBuffer* buffer,
	uint64_t offset,
	uint64_t size,
	void* data
);

std::unique_ptr<Mesh> Mesh::create(
	const Device* device,
	vk::CommandPool command_pool,
	vk::Queue queue,
	Shader* shader,
	std::string name,
	std::vector<vertex> vertices,
	std::vector<uint32_t> indices,
	vector4f diffuse_color,
	const Texture* diffuse_texture
)
{
	auto out = std::make_unique<Mesh>();

	// Copy trivial data.
	out->name = name;
	out->vertices = vertices;
	out->indices = indices;
	out->diffuse_texture = diffuse_texture;
	out->instance_ubo.diffuse_color = diffuse_color;
	out->shader = shader;
	out->device = device;

	uint64_t index_array_size = indices.size() * sizeof(uint32_t);
	uint64_t vertex_array_size = vertices.size() * sizeof(vertex);

	// Create the vertex buffer.
	out->vertex_buffer = VulkanBuffer::create(
		device,
		vertex_array_size,
		vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst |
			vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		true
	);

	if (!out->vertex_buffer)
	{
		sl::log_error("Failed to create vertex buffer for mesh `{}`.", name);
		return nullptr;
	}

	// Create the index buffer.
	out->index_buffer = VulkanBuffer::create(
		device,
		index_array_size,
		vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst |
			vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		true
	);

	if (!out->index_buffer)
	{
		sl::log_error("Failed to create index buffer for mesh `{}`.", name);
		return nullptr;
	}

	// Upload data to buffers.
	upload_data_range(
		device,
		command_pool,
		queue,
		out->vertex_buffer.get(),
		0,
		vertex_array_size,
		vertices.data()
	);

	upload_data_range(
		device,
		command_pool,
		queue,
		out->index_buffer.get(),
		0,
		index_array_size,
		indices.data()
	);

	// Create shader instance.
	out->shader_instance = shader->allocate_instance();

	if (!out->shader_instance)
	{
		sl::log_error("Failed to allocate a shader instance for mesh `{}`.", name);
		return nullptr;
	}

	// Update shader instance.
	out->shader_instance->set_ubo(&out->instance_ubo);
	out->shader_instance->set_sampler(0, out->diffuse_texture);

	return out;
}

Mesh::~Mesh()
{
	// TODO: free shader_instance.
}

void Mesh::draw(CommandBuffer* command_buffer, const mat4x4& model, uint32_t current_image)
{
	// Push the transformation matrix as a push constant.
	command_buffer->handle.pushConstants(
		shader->pipeline->pipeline_layout,
		vk::ShaderStageFlagBits::eVertex,
		0,
		64,
		&model
	);

	// Update the uniforms (if needed).
	shader_instance->update_ubo(current_image);

	// Set the instance descriptor sets.
	shader_instance->bind_descriptor_set(command_buffer, current_image);

	// Use the shader.
	shader->use(command_buffer, current_image);

	// Bind buffers.
	vk::DeviceSize offsets[1] = {0};
	command_buffer->handle.bindVertexBuffers(0, 1, &vertex_buffer->handle, offsets);

	command_buffer->handle.bindIndexBuffer(index_buffer->handle, 0, vk::IndexType::eUint32);

	// Issue draw call.
	command_buffer->handle.drawIndexed(indices.size(), 1, 0, 0, 0);
}

// Static helper functions.
static void upload_data_range(
	const Device* device,
	vk::CommandPool command_pool,
	vk::Queue queue,
	VulkanBuffer* buffer,
	uint64_t offset,
	uint64_t size,
	void* data
)
{
	// Create a host-visible staging buffer to upload to. Mark it as the source of the transfer.
	vk::MemoryPropertyFlags flags =
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

	auto staging = VulkanBuffer::create(
		device,
		size,
		vk::BufferUsageFlagBits::eTransferSrc,
		flags,
		true
	);

	// Load the data into the staging buffer.
	staging->load_data(0, size, {}, data);

	// Perform the copy from staging to the device local buffer.
	staging->copy_to(
		command_pool,
		nullptr,
		queue,
		0,
		buffer->handle,
		offset,
		size
	);
}

}
