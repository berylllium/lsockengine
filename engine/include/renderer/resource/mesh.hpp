#pragma once

#include <string>
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
	Mesh(
		const Device* device,
		VkCommandPool command_pool,
		VkQueue queue,
		Shader* shader,
		std::string name,
		std::vector<vertex> vertices,
		std::vector<uint32_t> indices,
		vector4f diffuse_color,
		const Texture* diffuse_texture
	);

	Mesh(Mesh&& other);

	Mesh(const Mesh&) = delete;

	~Mesh();

	Mesh& operator = (Mesh&& other);

	Mesh& operator = (const Mesh&) = delete;

	void draw(CommandBuffer& command_buffer, mat4x4 model, uint32_t current_image);

private:
	/**
	 * @brief The name of the mesh.
	 */
	std::string name;

	std::vector<vertex> vertices;

	std::vector<uint32_t> indices;

	VulkanBuffer* vertex_buffer;
	VulkanBuffer* index_buffer;

	MeshInstanceUBO instance_ubo;
	Shader::Instance* shader_instance;

	/**
	 * @brief A pointer to a loaded material in the lise_obj struct.
	 */
	const Texture* diffuse_texture;

	Shader* shader;

	const Device* device;
};

}
