#pragma once

#include <vector>

#include "definitions.hpp"
#include "renderer/resource/mesh.hpp"
#include "renderer/command_buffer.hpp"
#include "renderer/device.hpp"
#include "math/transform.hpp"
#include "renderer/resource/shader.hpp"
#include "loader/obj_loader.hpp"

namespace lise
{

struct Model
{
	Transform transform;

	std::vector<std::unique_ptr<Mesh>> meshes;

	Shader* shader;

	const Device* device;

	Model() = default;

	Model(Model&) = delete;

	Model& operator = (Model&) = delete;

	static std::unique_ptr<Model> create(const Device* device, Shader* shader, const Obj& obj);

	void draw(CommandBuffer* command_buffer, uint32_t current_image);
};

}
