#include "renderer/resource/mesh.h"

#include <stdlib.h>
#include <string.h>

#include "core/logger.h"

void static upload_data_range(
	VkDevice device,
	VkPhysicalDeviceMemoryProperties memory_properties,
	VkCommandPool command_pool,
	VkQueue queue,
	lise_vulkan_buffer* buffer,
	uint64_t offset,
	uint64_t size,
	void* data
);

bool lise_mesh_create(
	VkDevice device,
	VkPhysicalDeviceMemoryProperties memory_properties,
	VkCommandPool command_pool,
	VkQueue queue,
	lise_shader* shader,
	char* name,
	uint32_t vertex_count,
	lise_vertex* vertices,
	uint32_t index_count,
	uint32_t* indices,
	lise_vec4 diffuse_color,
	lise_texture* diffuse_texture,
	lise_mesh* out_mesh
)
{
	// Clear out_mesh.
	memset(out_mesh, 0, sizeof(lise_mesh));

	// Make a copy of the name.
	out_mesh->name = strdup(name);

	// Make a copy of the vertices.
	out_mesh->vertex_count = vertex_count;

	uint64_t vertex_array_size = vertex_count * sizeof(lise_vertex);
	out_mesh->vertices = malloc(vertex_array_size);
	memcpy(out_mesh->vertices, vertices, vertex_array_size);

	// Make a copy of the indices.
	out_mesh->index_count = index_count;

	uint64_t index_array_size = index_count * sizeof(uint32_t);
	out_mesh->indices = malloc(index_array_size);
	memcpy(out_mesh->indices, indices, index_array_size);

	// Copy over the diffuse texture pointer.
	out_mesh->diffuse_texture = diffuse_texture;

	
	// Create the vertex buffer.
	if (!lise_vulkan_buffer_create(
		device,
		memory_properties,
		vertex_array_size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		true,
		&out_mesh->vertex_buffer
	))
	{
		LERROR("Failed to create vertex buffer for mesh `%s`.", name);

		lise_mesh_free(device, out_mesh);
		return false;
	}

	// Create the index buffer.
	if (!lise_vulkan_buffer_create(
		device,
		memory_properties,
		index_array_size,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		true,
		&out_mesh->index_buffer
	))
	{
		LERROR("Failed to create index buffer for mesh `%s`.", name);

		lise_mesh_free(device, out_mesh);
		return false;
	}

	// Upload data to buffers.
	upload_data_range(
		device,
		memory_properties,
		command_pool,
		queue,
		&out_mesh->vertex_buffer,
		0,
		vertex_array_size,
		out_mesh->vertices
	);

	upload_data_range(
		device,
		memory_properties,
		command_pool,
		queue,
		&out_mesh->index_buffer,
		0,
		index_array_size,
		out_mesh->indices
	);

	// Create shader instance.
	if (!lise_shader_allocate_instance(device, shader, &out_mesh->shader_instance))
	{
		LERROR("Failed to allocate a shader instance for mesh `%s`.", name);
		return false;
	}

	// Populate ubo.
	out_mesh->instance_ubo.diffuse_color = diffuse_color;

	// Update shader instance.
	lise_shader_set_instance_ubo(device, shader, &out_mesh->instance_ubo, &out_mesh->shader_instance);
	lise_shader_set_instance_sampler(device, shader, 0, out_mesh->diffuse_texture, &out_mesh->shader_instance);

	return true;
}

void lise_mesh_free(VkDevice device, lise_mesh* mesh)
{
	lise_vulkan_buffer_destroy(device, &mesh->vertex_buffer);
	lise_vulkan_buffer_destroy(device, &mesh->index_buffer);

	free(mesh->name);
	free(mesh->vertices);
	free(mesh->indices);

	memset(mesh, 0, sizeof(lise_mesh));
}

void lise_mesh_draw(
	VkDevice device,
	lise_mesh* mesh,
	lise_shader* shader,
	VkCommandBuffer command_buffer,
	lise_mat4x4 model,
	uint32_t current_image
)
{
	// Push the transformation matrix as a push constant.
	vkCmdPushConstants(command_buffer, shader->pipeline.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, 64, &model);

	// Update the uniforms (if needed).
	lise_shader_update_instance_ubo(device, shader, current_image, &mesh->shader_instance);

	// Set the instance descriptor sets.
	vkCmdBindDescriptorSets(
		command_buffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		shader->pipeline.pipeline_layout,
		1,
		1,
		&mesh->shader_instance.descriptor_sets[current_image],
		0,
		0
	);

	// Use the shader.
	lise_shader_use(shader, command_buffer, current_image);

	// Bind buffers.
	VkDeviceSize offsets[1] = {0};
	vkCmdBindVertexBuffers(command_buffer, 0, 1, &mesh->vertex_buffer.handle, offsets);

	vkCmdBindIndexBuffer(command_buffer, mesh->index_buffer.handle, 0, VK_INDEX_TYPE_UINT32);

	// Issue draw call.
	vkCmdDrawIndexed(command_buffer, mesh->index_count, 1, 0, 0, 0);
}

// Static helper functions.
void static upload_data_range(
	VkDevice device,
	VkPhysicalDeviceMemoryProperties memory_properties,
	VkCommandPool command_pool,
	VkQueue queue,
	lise_vulkan_buffer* buffer,
	uint64_t offset,
	uint64_t size,
	void* data
)
{
	// Create a host-visible staging buffer to upload to. Mark it as the source of the transfer.
	VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	lise_vulkan_buffer staging;
	lise_vulkan_buffer_create(
		device,
		memory_properties,
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		flags,
		true,
		&staging
	);

	// Load the data into the staging buffer.
	lise_vulkan_buffer_load_data(device, &staging, 0, size, 0, data);

	// Perform the copy from staging to the device local buffer.
	lise_vulkan_buffer_copy_to(
		device,
		command_pool,
		0,
		queue,
		staging.handle,
		0,
		buffer->handle,
		offset,
		size
	);

	// Clean up the staging buffer.
	lise_vulkan_buffer_destroy(device, &staging);
}
