#include "renderer/resource/model.hpp"

#include "renderer/system/texture_system.hpp"

#include <simple-logger.hpp>

namespace lise
{
	
std::unique_ptr<Model> Model::create(const Device* device, Shader* shader, const Obj& obj)
{
	auto out = std::make_unique<Model>();

	// Allocate meshes.
	out->device = device;
	out->shader = shader;

	out->meshes.reserve(obj.meshes.size());

	// Prase meshes.
	for (uint32_t i = 0; i < obj.meshes.size(); i++)
	{
		const Texture* loaded_texture = texture_system_get_or_load(device, obj.meshes[i].material->map_Kd);

		vector3f Kd = obj.meshes[i].material->Kd;
//		vec4 diffuse_color = (vec4) { Kd.r, Kd.g, Kd.b, 1.0f };
		vector4f diffuse_color = { 1.0f, 1.0f, 1.0f, 1.0f }; // TODO: Put this back to configurable.

		auto m = Mesh::create(
			device,
			device->graphics_command_pool,
			device->graphics_queue,
			shader,
			obj.meshes[i].name,
			obj.meshes[i].vertices,
			obj.meshes[i].indices,
			diffuse_color,
			loaded_texture
		);

		if (!m)
		{
			sl::log_error("Failed to create mesh.");

			return nullptr;
		}

		out->meshes.push_back(std::move(m));
	}

	return out;
}

void Model::draw(CommandBuffer* command_buffer, uint32_t current_image)
{
	for (size_t i = 0; i < meshes.size(); i++)
	{
		meshes[i]->draw(command_buffer, transform.get_transformation_matrix(), current_image);
	}
}

}
