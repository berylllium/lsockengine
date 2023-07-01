#pragma once 

#include <unordered_map>
#include <string>
#include <vector>

#include "definitions.hpp"
#include "renderer/vulkan_buffer.hpp"
#include "renderer/pipeline.hpp"
#include "renderer/resource/texture.hpp"

#include "loader/shader_config_loader.hpp"

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
	class Instance
	{
		friend class Shader;

	public:
		Instance(Instance&& other);
	
		Instance(const Instance&) = delete; // Prevent copies.
	
		~Instance();
	
		Instance& operator = (const Instance&) = delete; // Prevent copies.
		
		Instance& operator = (Instance&& other);
	
		void set_ubo(void* data);
	
		void set_sampler(uint32_t sampler_index, const Texture* sampler);
	
		void update_ubo(uint32_t current_image);

		void bind_descriptor_set(CommandBuffer& command_buffer, uint32_t current_image);

	private:
		Instance() = default;

		uint64_t id;
	
		std::vector<VkDescriptorSet> descriptor_sets;
	
		void* ubo; // NOTE: Maybe make this constant?
		std::vector<bool> ubo_dirty;
	
		/**
		 * @brief An array of texture pointers.
		 */
		std::vector<const Texture*> samplers;
	
		/**
		 * @brief An array of booleans representing if the samplers are dirty. Theres `swapchain_image_count` booleans per
		 * sampler.
		 */
		std::vector<bool> sampler_dirty;
	
		const Shader* shader;
	};

	Shader(
		const Device& device,
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
	
	void set_global_ubo(void* data);

	void update_global_uniforms(uint32_t current_image);

	const Pipeline& get_pipeline() const;

	Instance* allocate_instance();

	void deallocate_instance(uint64_t id);

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

	/**
	 * @brief An array of vertex attributes used by the input assembler.
	 */
	std::vector<ShaderAttribute> vertex_attributes;

	// Global uniform data.
	VkDescriptorPool global_descriptor_pool;
	VkDescriptorSetLayout global_descriptor_set_layout;

	/**
	 * @brief An array of global descriptor sets. These point to the same resource, but are updated seperately to
	 * prevent updating a descriptor set mid render pass.
	 */
	std::vector<VkDescriptorSet> global_descriptor_sets;

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

	std::vector<bool> global_ubo_dirty;

	/**
	 * @brief An array of all the global uniforms.
	 * 
	 * One uniform per swapchain image.
	 */
	std::vector<ShaderUniform> global_uniforms;

	// Instance uniform data.
	VkDescriptorPool instance_descriptor_pool;
	VkDescriptorSetLayout instance_descriptor_set_layout;

	/**
	 * @brief The instance uniform buffer.
	 */
	VulkanBuffer* instance_ub;

	/**
	 * @brief An array of booleans representing free slots in the instance uniform buffer.
	 */
	std::vector<bool> instance_ubo_free_list;

	std::unordered_map<uint64_t, Instance> instances;

	std::vector<ShaderUniform> instance_uniforms;

	std::vector<ShaderUniform> instance_samplers;;

	uint32_t instance_ubo_size;
	uint32_t instance_ubo_stride;

	Pipeline* pipeline;

	const Device& device;
};

}
