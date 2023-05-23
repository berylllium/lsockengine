#pragma once 

#include <string>
#include <memory>

#include "definitions.hpp"
#include "renderer/vulkan_buffer.hpp"
#include "renderer/pipeline.hpp"
#include "renderer/resource/texture.hpp"

#define LSHADER_MAX_INSTANCE_COUNT 1024

namespace lise
{

enum class ShaderScope
{
	GLOBAL,
	INSTANCE,
	LOCAL
};

enum class ShaderUniformType
{
	FLOAT32,
	FLOAT32_2,
	FLOAT32_3,
	FLOAT32_4,
	INT8,
	UINT8,
	INT16,
	UINT16,
	INT32,
	UINT32,
	MATRIX_4,
	SAMPLER,

	CUSTOM = 255U
};

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

struct ShaderUniform
{
	std::string name;

	/**
	 * @brief The offset from the beginning of the uniform buffer object.
	 */
	uint64_t offset;

	/**
	 * @brief The set index of the descriptor.
	 */
	uint8_t index;

	ShaderUniformType type;

	uint32_t size;

	ShaderScope scope;
};

struct ShaderAttribute
{
	std::string name;

	VkFormat type;

	uint32_t size;
};

class Shader
{
public:
	Shader(
		const Device& device,
		VkPhysicalDeviceMemoryProperties memory_properties,
		uint64_t minimum_uniform_alignment,
		const ShaderConfig& shader_config,
		const RenderPass& render_pass, // TODO: Temporary, add a render pass system.
		uint32_t framebuffer_width,
		uint32_t framebuffer_height,
		uint32_t swapchain_image_count
	);

	Shader(Shader&& other);

	Shader(const Shader&) = delete; // Prevent copies.

	~Shader();

	Shader& operator = (const Shader&) = delete; // Prevent copies.

	void use(CommandBuffer& command_buffer, uint32_t current_image);

	void update_global_uniforms(uint32_t current_image);

	void set_global_ubo(void* data);


private:
	/**
	 * @brief The internal identifier of the shader.
	 */
	uint32_t id;

	/**
	 * @brief The name of the shader. This string is also used in the hashtable.
	 */
	std::string name;
	
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
	std::unique_ptr<ShaderAttribute[]> vertex_attributes;

	// Global uniform data.
	VkDescriptorPool global_descriptor_pool;
	VkDescriptorSetLayout global_descriptor_set_layout;

	/**
	 * @brief An array of global descriptor sets. These point to the same resource, but are updated seperately to
	 * prevent updating a descriptor set mid render pass.
	 */
	std::unique_ptr<VkDescriptorSet[]> global_descriptor_sets;

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
	VulkanBuffer* global_ub;

	std::unique_ptr<bool[]> global_ubo_dirty;

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
	std::unique_ptr<ShaderUniform[]> global_uniforms;

	// Instance uniform data.
	VkDescriptorPool instance_descriptor_pool;
	VkDescriptorSetLayout instance_descriptor_set_layout;

	/**
	 * @brief The instance uniform buffer.
	 */
	VulkanBuffer* instance_ub;

	/**
	 * @brief An array of booleans representing free slots in the instance uniform buffer.
	 * 
	 */
	std::unique_ptr<bool[]> instance_ubo_free_list;

	uint32_t instance_uniform_count;
	std::unique_ptr<ShaderUniform[]> instance_uniforms;

	uint32_t instance_sampler_count;
	std::unique_ptr<ShaderUniform[]> instance_samplers;;

	uint32_t instance_ubo_size;
	uint32_t instance_ubo_stride;

	Pipeline* pipeline;

	const Device& device;

	// Friends.
	friend class ShaderInstance;
};

class ShaderInstance
{
public:
	ShaderInstance(const Shader& shader);

	ShaderInstance(ShaderInstance&& other);

	ShaderInstance(const ShaderInstance&) = delete; // Prevent copies.

	~ShaderInstance();

	ShaderInstance& operator = (const ShaderInstance&) = delete; // Prevent copies.

	void set_ubo(void* data);

	void set_sampler(uint32_t sampler_index, const Texture* sampler);

	void update_ubo(uint32_t current_image);

private:
	uint64_t id;

	std::unique_ptr<VkDescriptorSet[]> descriptor_sets;

	void* ubo; // NOTE: Maybe make this constant?
	std::unique_ptr<bool[]> ubo_dirty;

	/**
	 * @brief An array of texture pointers.
	 */
	std::unique_ptr<const Texture*[]> samplers;

	/**
	 * @brief An array of booleans representing if the samplers are dirty. Theres `swapchain_image_count` booleans per
	 * sampler.
	 */
	std::unique_ptr<bool[]> sampler_dirty;

	const Shader& shader;
};

}
