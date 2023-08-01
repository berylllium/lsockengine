#include "renderer/resource/mesh.hpp"

#include <format>

#include <simple-logger.hpp>

namespace lise
{

static void upload_data_range(
	const Device& device,
	VkCommandPool command_pool,
	VkQueue queue,
	VulkanBuffer& buffer,
	uint64_t offset,
	uint64_t size,
	void* data
);

Mesh::Mesh(
	const Device* device,
	VkCommandPool command_pool,
	VkQueue queue,
	Shader* shader,
	std::string name,
	std::vector<vertex> vertices,
	std::vector<uint32_t> indices,
	vector4f diffuse_color,
	const Texture* diffuse_texture
) : device(device), shader(shader), name(name), vertices(vertices), indices(indices), diffuse_texture(diffuse_texture)
{
	uint64_t index_array_size = indices.size() * sizeof(uint32_t);
	uint64_t vertex_array_size = vertices.size() * sizeof(vertex);

	// Create the vertex buffer.
	vertex_buffer = new VulkanBuffer(
		*device,
		device->get_memory_properties(),
		vertex_array_size,
		static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		true
	);

	// Create the index buffer.
	index_buffer = new VulkanBuffer(
		*device,
		device->get_memory_properties(),
		index_array_size,
		static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		true
	);

	// Upload data to buffers.
	upload_data_range(
		*device,
		command_pool,
		queue,
		*vertex_buffer,
		0,
		vertex_array_size,
		vertices.data()
	);

	upload_data_range(
		*device,
		command_pool,
		queue,
		*index_buffer,
		0,
		index_array_size,
		indices.data()
	);

	// Create shader instance.
	shader_instance = shader->allocate_instance();
	if (shader_instance == nullptr)
	{
		sl::log_error("Failed to allocate a shader instance for mesh `%s`.", name);

		throw std::runtime_error("Failed to allocate a shader instance");
	}

	// Populate ubo.
	instance_ubo.diffuse_color = diffuse_color;

	// Update shader instance.
	shader_instance->set_ubo(&instance_ubo);
	shader_instance->set_sampler(0, diffuse_texture);
}

Mesh::Mesh(Mesh&& other)
{
	name = other.name;

	vertices = other.vertices;

	indices = other.indices;

	vertex_buffer = other.vertex_buffer;
	other.vertex_buffer = nullptr;

	index_buffer = other.index_buffer;
	other.index_buffer = nullptr;

	instance_ubo = other.instance_ubo;

	shader_instance = other.shader_instance;
	other.shader_instance = nullptr;

	diffuse_texture = other.diffuse_texture;

	shader = other.shader;
	
	device = other.device;
}

Mesh::~Mesh()
{
	delete vertex_buffer;
	delete index_buffer;

	// TODO: free shader_instance.
}

Mesh& Mesh::operator = (Mesh&& other)
{
	this->~Mesh();

	name = other.name;

	vertices = other.vertices;

	indices = other.indices;

	vertex_buffer = other.vertex_buffer;
	other.vertex_buffer = nullptr;

	index_buffer = other.index_buffer;
	other.index_buffer = nullptr;

	instance_ubo = other.instance_ubo;

	shader_instance = other.shader_instance;
	other.shader_instance = nullptr;

	diffuse_texture = other.diffuse_texture;

	shader = other.shader;
	
	device = other.device;

	return *this;
}

void Mesh::draw(CommandBuffer& command_buffer, mat4x4 model, uint32_t current_image)
{
	// Push the transformation matrix as a push constant.
	vkCmdPushConstants(command_buffer, shader->get_pipeline().get_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, 64, &model);

	// Update the uniforms (if needed).
	shader_instance->update_ubo(current_image);

	// Set the instance descriptor sets.
	shader_instance->bind_descriptor_set(command_buffer, current_image);

	// Use the shader.
	shader->use(command_buffer, current_image);

	// Bind buffers.
	VkDeviceSize offsets[1] = {0};
	VkBuffer vhandle = *vertex_buffer;
	vkCmdBindVertexBuffers(command_buffer, 0, 1, &vhandle, offsets);

	vkCmdBindIndexBuffer(command_buffer, *index_buffer, 0, VK_INDEX_TYPE_UINT32);

	// Issue draw call.
	vkCmdDrawIndexed(command_buffer, indices.size(), 1, 0, 0, 0);
}

// Static helper functions.
static void upload_data_range(
	const Device& device,
	VkCommandPool command_pool,
	VkQueue queue,
	VulkanBuffer& buffer,
	uint64_t offset,
	uint64_t size,
	void* data
)
{
	// Create a host-visible staging buffer to upload to. Mark it as the source of the transfer.
	VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	VulkanBuffer staging(
		device,
		device.get_memory_properties(),
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		flags,
		true
	);

	// Load the data into the staging buffer.
	staging.load_data(0, size, 0, data);

	// Perform the copy from staging to the device local buffer.
	staging.copy_to(
		command_pool,
		0,
		queue,
		0,
		buffer,
		offset,
		size
	);
}

}
