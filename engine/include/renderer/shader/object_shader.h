#pragma once

#include "definitions.h"
#include "renderer/shader_stage.h"
#include "renderer/pipeline.h"
#include "renderer/vulkan_buffer.h"
#include "math/mat4x4.h"

#define LOBJECT_SHADER_STAGE_COUNT 2

typedef struct lise_object_shader_global_ubo
{
	lise_mat4x4 projection;   // 64 bytes
	lise_mat4x4 view;         // 64 bytes
	lise_mat4x4 m_reserved0;  // 64 bytes, reserved for future use
	lise_mat4x4 m_reserved1;  // 64 bytes, reserved for future use
} lise_object_shader_global_ubo;

typedef struct lise_object_shader
{
	VkDescriptorPool global_descriptor_pool;
	VkDescriptorSetLayout global_descriptor_set_layout;

	// One descriptor set per swapchain image
	uint32_t global_descriptor_set_count;
	VkDescriptorSet* global_descriptor_sets;
	bool* global_descriptor_sets_updated;

	lise_object_shader_global_ubo global_ubo;

	lise_vulkan_buffer global_uniform_buffer;

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
