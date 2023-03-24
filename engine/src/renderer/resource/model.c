#include "renderer/resource/model.h"

#include <stdlib.h>
#include <string.h>

#include "core/logger.h"	
#include "loader/obj_loader.h"

#include "renderer/system/texture_system.h"

bool lise_model_load(lise_device* device, const char* path, lise_shader* shader, lise_model* out_model)
{
	// Clear out_model.
	memset(out_model, 0, sizeof(lise_model));

	// Create transform.
	out_model->transform = lise_transform_create();

	// Create shader instance.
	out_model->shader = shader;
	
	// Load obj file.
	lise_obj loaded_obj;

	if (!lise_obj_load(path, &loaded_obj))
	{
		LFATAL("Failed to load model `%s`.", path);

		return false;
	}

	// Allocate meshes.
	out_model->mesh_count = loaded_obj.mesh_count;
	out_model->meshes = calloc(out_model->mesh_count, sizeof(lise_mesh));

	// Prase meshes.
	for (uint32_t i = 0; i < loaded_obj.mesh_count; i++)
	{
		lise_texture* loaded_texture;
		lise_texture_system_get_or_load(device, loaded_obj.meshes[i].material->map_Kd, &loaded_texture);

		lise_vec3 Kd = loaded_obj.meshes[i].material->Kd;
//		lise_vec4 diffuse_color = (lise_vec4) { Kd.r, Kd.g, Kd.b, 1.0f };
		lise_vec4 diffuse_color = (lise_vec4) { 1.0f, 1.0f, 1.0f, 1.0f }; // TODO: Put this back to configurable.

		if (!lise_mesh_create(
			device->logical_device,
			device->physical_device_memory_properties,
			device->graphics_command_pool,
			device->graphics_queue,
			shader,
			loaded_obj.meshes[i].name,
			loaded_obj.meshes[i].vertex_count,
			loaded_obj.meshes[i].vertices,
			loaded_obj.meshes[i].index_count,
			loaded_obj.meshes[i].indices,
			diffuse_color,
			loaded_texture,
			&out_model->meshes[i]
		))
		{
			LERROR("Failed to create mesh `%s` for model `%s`.", loaded_obj.meshes[i].name, path);

			lise_model_free(device->logical_device, out_model);
			lise_obj_free(&loaded_obj);
			return false;
		}
	}
}

void lise_model_free(VkDevice device, lise_model* model)
{
	// Free meshes.
	for (uint32_t i = 0; i < model->mesh_count; i++)
	{
		lise_mesh_free(device, &model->meshes[i]);
	}

	free(model->meshes);

	// Free transform.
	lise_transform_free(&model->transform);
}

void lise_model_draw(lise_model* model, VkDevice device, VkCommandBuffer command_buffer, uint32_t current_image)
{
	for (uint64_t i = 0; i < model->mesh_count; i++)
	{
		lise_mesh_draw(
			device,
			&model->meshes[i],
			model->shader,
			command_buffer,
			model->transform.transformation_matrix,
			current_image
		);
	}
}
