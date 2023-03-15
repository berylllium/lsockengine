#pragma once

#include "definitions.h"
#include "renderer/shader_stage.h"
#include "renderer/pipeline.h"
#include "renderer/vulkan_buffer.h"
#include "math/mat4x4.h"
#include "math/vector4.h"
#include "renderer/resource/texture.h"

#define LOBJECT_SHADER_STAGE_COUNT 2
#define LOBJECT_SHADER_MAX_OBJECT_COUNT 1024
#define LOBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT 2
#define LOBJECT_SHADER_SAMPLER_PER_OBJECT_COUNT 1

typedef struct lise_object_shader_object_render_packet
{
	uint32_t id;
	lise_mat4x4 model_matrix;
	lise_texture* texture;
} lise_object_shader_object_render_packet;

// Uniform buffer objects
typedef struct lise_object_shader_global_ubo
{
	lise_mat4x4 projection;		// 64 bytes
	lise_mat4x4 view;			// 64 bytes
	lise_mat4x4 m_reserved0;	// 64 bytes, reserved for future use
	lise_mat4x4 m_reserved1;	// 64 bytes, reserved for future use
} lise_object_shader_global_ubo;

typedef struct lise_object_shader_object_ubo
{
	lise_vec4 diffuse_color;	// 16 bytes
	lise_vec4 v_reserved0;		// 16 bytes, reserved for future use
	lise_vec4 v_reserved1;		// 16 bytes, reserved for future use
	lise_vec4 v_reserved2;		// 16 bytes, reserved for future use
} lise_object_shader_object_ubo;

typedef struct lise_object_shader_object
{
	uint32_t ub_index;

	// Per swapchain image.
	uint32_t descriptor_set_count;
	VkDescriptorSet* descriptor_sets;

	// Per descriptor per swapchain image.
	bool* is_descriptor_dirty;

	// Data
	lise_object_shader_object_ubo object_ubo;
	const lise_texture* diffuse_texture;

} lise_object_shader_object;

typedef struct lise_object_shader
{
	VkDescriptorPool global_descriptor_pool;
	VkDescriptorSetLayout global_descriptor_set_layout;

	// One per swapchain image.
	uint32_t descriptor_set_count;
	VkDescriptorSet* global_descriptor_sets;
	bool* global_descriptor_sets_updated;

	lise_object_shader_global_ubo global_ubo;

	lise_vulkan_buffer global_uniform_buffer;

	// Object
	VkDescriptorPool object_descriptor_pool;
	VkDescriptorSetLayout object_descriptor_set_layout;

	lise_vulkan_buffer object_uniform_buffer;
	uint32_t current_uniform_buffer_index;

	lise_pipeline pipeline;
} lise_object_shader;

bool lise_object_shader_create(
	VkDevice device,
	VkPhysicalDeviceMemoryProperties memory_properties,
	lise_render_pass* render_pass,
	uint32_t framebuffer_width,
	uint32_t framebuffer_height,
	uint32_t swapchain_image_count,
	lise_object_shader* out_object_shader
);

void lise_object_shader_destroy(VkDevice device, lise_object_shader* object_shader);

void lise_object_shader_use(uint32_t image_index, VkCommandBuffer command_buffer, lise_object_shader* object_shader);

void lise_object_shader_set_global_ubo(lise_object_shader_global_ubo new_global_ubo, lise_object_shader* object_shader);

void lise_object_shader_update_global_state(
	VkDevice device,
	lise_object_shader* object_shader,
	uint32_t image_index
);

bool lise_object_shader_register_object(
	lise_object_shader* object_shader, 
	VkDevice device,
	uint32_t swapchain_image_count,
	lise_object_shader_object* out_object
);

bool lise_object_shader_free_object(
	VkDevice device,
	lise_object_shader* object_shader,
	lise_object_shader_object* object
);

void lise_object_shader_set_object_data(
	lise_object_shader_object* object,
	lise_object_shader_object_ubo* object_ubo,
	lise_texture* diffuse_texture
);

void lise_object_shader_update_object(
	lise_object_shader_object* object,
	lise_object_shader* object_shader,
	VkCommandBuffer command_buffer,
	VkDevice device,
	uint32_t swapchain_image_index
);
