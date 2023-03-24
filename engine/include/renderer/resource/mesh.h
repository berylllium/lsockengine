#pragma once

#include "definitions.h"
#include "math/vertex.h"
#include "renderer/resource/texture.h"
#include "renderer/vulkan_buffer.h"
#include "renderer/shader.h"
#include "math/vector4.h"
#include "math/mat4x4.h"

typedef struct lise_mesh_instance_ubo
{
	lise_vec4 diffuse_color;
} lise_mesh_instance_ubo;

typedef struct lise_mesh
{
	/**
	 * @brief The name of the mesh.
	 */
	char* name;

	uint32_t vertex_count;
	lise_vertex* vertices;

	uint32_t index_count;
	uint32_t* indices;

	lise_vulkan_buffer vertex_buffer;
	lise_vulkan_buffer index_buffer;

	lise_mesh_instance_ubo instance_ubo;
	lise_shader_instance shader_instance;

	/**
	 * @brief A pointer to a loaded material in the lise_obj struct.
	 */
	lise_texture* diffuse_texture;
} lise_mesh;

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
);

void lise_mesh_free(VkDevice device, lise_mesh* mesh);

void lise_mesh_draw(
	VkDevice device,
	lise_mesh* mesh,
	lise_shader* shader,
	VkCommandBuffer command_buffer,
	lise_mat4x4 model,
	uint32_t current_image
);
