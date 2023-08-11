#pragma once

#include <string>
#include <memory>
#include <vector>

#include "definitions.hpp"
#include "math/vertex.hpp"
#include "renderer/command_buffer.hpp"
#include "renderer/resource/shader.hpp"
#include "renderer/resource/texture.hpp"
#include "renderer/vulkan_buffer.hpp"
#include "math/vector4.hpp"
#include "math/mat4x4.hpp"

namespace lise
{

struct MeshInstanceUBO
{
	vector4f diffuse_color;
};

class Mesh
{
public:
	std::string name;

	std::vector<vertex> vertices;

	std::vector<uint32_t> indices;

	std::unique_ptr<VulkanBuffer> vertex_buffer;
	std::unique_ptr<VulkanBuffer> index_buffer;

	MeshInstanceUBO instance_ubo;
	Shader::Instance* shader_instance;

	/**
	 * @brief A pointer to a loaded material in the lise_obj struct.
	 */
	const Texture* diffuse_texture;

	Shader* shader;

	const Device* device;

	Mesh() = default;

	Mesh(const Mesh&) = delete;

	~Mesh();

	Mesh& operator = (const Mesh&) = delete;

	static std::unique_ptr<Mesh> create(
		const Device* device,
		vk::CommandPool command_pool,
		vk::Queue queue,
		Shader* shader,
		std::string name,
		std::vector<vertex> vertices,
		std::vector<uint32_t> indices,
		vector4f diffuse_color,
		const Texture* diffuse_texture
	);

	void draw(CommandBuffer* command_buffer, const mat4x4& model, uint32_t current_image);
};

}
