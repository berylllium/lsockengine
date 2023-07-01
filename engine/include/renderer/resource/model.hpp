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

class Model
{
public:
	Model(const Device* device, Shader* shader, const Obj& obj);

	void draw(CommandBuffer& command_buffer, uint32_t current_image);

	Transform& get_transform();

private:
	Transform transform;

	Shader* shader;

	std::vector<Mesh> meshes;

	const Device* device;
};

}
