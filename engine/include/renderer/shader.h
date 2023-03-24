#pragma once 

#include "definitions.h"
#include "renderer/vulkan_buffer.h"
#include "renderer/pipeline.h"
#include "renderer/resource/texture.h"

#define LSHADER_MAX_INSTANCE_COUNT 1024

typedef enum lise_shader_scope
{
	LISE_SHADER_SCOPE_GLOBAL,
	LISE_SHADER_SCOPE_INSTANCE,
	LISE_SHADER_SCOPE_LOCAL
} lise_shader_scope;

typedef enum lise_shader_uniform_type
{
	LISE_SHADER_UNIFORM_TYPE_FLOAT32,
	LISE_SHADER_UNIFORM_TYPE_FLOAT32_2,
	LISE_SHADER_UNIFORM_TYPE_FLOAT32_3,
	LISE_SHADER_UNIFORM_TYPE_FLOAT32_4,
	LISE_SHADER_UNIFORM_TYPE_INT8,
	LISE_SHADER_UNIFORM_TYPE_UINT8,
	LISE_SHADER_UNIFORM_TYPE_INT16,
	LISE_SHADER_UNIFORM_TYPE_UINT16,
	LISE_SHADER_UNIFORM_TYPE_INT32,
	LISE_SHADER_UNIFORM_TYPE_UINT32,
	LISE_SHADER_UNIFORM_TYPE_MATRIX_4,
	LISE_SHADER_UNIFORM_TYPE_SAMPLER,

	LISE_SHADER_UNIFORM_TYPE_CUSTOM = 255U
} lise_shader_uniform_type;

//typedef enum lise_shader_attribute_type
//{
//	LISE_SHADER_ATTRIB_TYPE_FLOAT32,
//	LISE_SHADER_ATTRIB_TYPE_FLOAT32_2,
//	LISE_SHADER_ATTRIB_TYPE_FLOAT32_3,
//	LISE_SHADER_ATTRIB_TYPE_FLOAT32_4,
//	LISE_SHADER_ATTRIB_TYPE_MATRIX_4,
//	LISE_SHADER_ATTRIB_TYPE_INT8,
//	LISE_SHADER_ATTRIB_TYPE_UINT8,
//	LISE_SHADER_ATTRIB_TYPE_INT16,
//	LISE_SHADER_ATTRIB_TYPE_UINT16,
//	LISE_SHADER_ATTRIB_TYPE_INT32,
//	LISE_SHADER_ATTRIB_TYPE_UINT32
//} lise_shader_attribute_type;

typedef struct lise_shader_uniform
{
	char* name;

	/**
	 * @brief The offset from the beginning of the uniform buffer object.
	 */
	uint64_t offset;

	/**
	 * @brief The set index of the descriptor.
	 */
	uint8_t index;

	lise_shader_uniform_type type;

	uint32_t size;

	lise_shader_scope scope;
} lise_shader_uniform;

typedef struct lise_shader_attribute
{
	char* name;

	VkFormat type;

	uint32_t size;
} lise_shader_attribute;

typedef struct lise_shader_instance
{
	uint64_t id;

	VkDescriptorSet* descriptor_sets;

	void* ubo;
	bool* ubo_dirty;

	/**
	 * @brief An array of texture pointers.
	 */
	const lise_texture** samplers;

	/**
	 * @brief An array of booleans representing if the samplers are dirty. Theres `swapchain_image_count` booleans per
	 * sampler.
	 */
	bool* sampler_dirty;
} lise_shader_instance;

typedef struct lise_shader
{
	/**
	 * @brief The internal identifier of the shader.
	 */
	uint32_t id;

	/**
	 * @brief The name of the shader. This string is also used in the hashtable.
	 */
	char* name;
	
	uint64_t minimum_uniform_alignment;
	
	/**
	 * @brief A copy of the swapchain image count created during shader creation. This value is used during indexing
	 * of descriptor sets and uniform buffers.
	 */
	uint32_t swapchain_image_count;

	uint32_t vertex_attribute_count;

	/**
	 * @brief An array of vertex attributes used by the input assembler.
	 */
	lise_shader_attribute* vertex_attributes;

	// Global uniform data.
	VkDescriptorPool global_descriptor_pool;
	VkDescriptorSetLayout global_descriptor_set_layout;

	/**
	 * @brief An array of global descriptor sets. These point to the same resource, but are updated seperately to
	 * prevent updating a descriptor set mid render pass.
	 */
	VkDescriptorSet* global_descriptor_sets;

	uint64_t global_ubo_size;

	/**
	 * @brief The stride of the ubo in the buffer. This is not the same as the UBOs size as many graphics cards require
	 * a minimum offset alignment.
	 */
	uint64_t global_ubo_stride;

	void* global_ubo;

	/**
	 * @brief The global uniform buffer object.
	 */
	lise_vulkan_buffer global_ub;

	bool* global_ubo_dirty;

	/**
	 * @brief The amount of global uniforms in the buffer. This is excluding the swapchain image influence.
	 * 
	 */
	uint32_t global_uniform_count;

	/**
	 * @brief An array of all the global uniforms.
	 * 
	 * One uniform per swapchain image.
	 */
	lise_shader_uniform* global_uniforms;

	// Instance uniform data.
	VkDescriptorPool instance_descriptor_pool;
	VkDescriptorSetLayout instance_descriptor_set_layout;

	/**
	 * @brief The instance uniform buffer.
	 */
	lise_vulkan_buffer instance_ub;

	/**
	 * @brief An array of booleans representing free slots in the instance uniform buffer.
	 * 
	 */
	bool* instance_ubo_free_list;

	uint32_t instance_uniform_count;
	lise_shader_uniform* instance_uniforms;

	uint32_t instance_sampler_count;
	lise_shader_uniform* instance_samplers;;

	uint32_t instance_ubo_size;
	uint32_t instance_ubo_stride;

	lise_pipeline pipeline;
} lise_shader;

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
);

void lise_shader_destroy(VkDevice device, lise_shader* shader);

void lise_shader_use(lise_shader* shader, VkCommandBuffer command_buffer, uint32_t current_image);

bool lise_shader_allocate_instance(VkDevice device, lise_shader* shader, lise_shader_instance* out_instance);

void lise_shader_free_instance(VkDevice device, lise_shader* shader, lise_shader_instance* instance);

void lise_shader_set_global_ubo(VkDevice device, lise_shader* shader, void* data);

void lise_shader_update_global_uniforms(
	VkDevice device,
	lise_shader* shader,
	uint32_t current_image
);

void lise_shader_set_instance_ubo(
	VkDevice device,
	lise_shader* shader,
	void* data,
	lise_shader_instance* instance
);

void lise_shader_set_instance_sampler(
	VkDevice device,
	lise_shader* shader,
	uint32_t sampler_index,
	lise_texture* sampler,
	lise_shader_instance* instance
);

bool lise_shader_update_instance_ubo(
	VkDevice device,
	lise_shader* shader,
	uint32_t current_image,
	lise_shader_instance* instance
);
