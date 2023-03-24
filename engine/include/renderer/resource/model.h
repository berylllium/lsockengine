#pragma once

#include "definitions.h"
#include "renderer/resource/mesh.h"
#include "renderer/device.h"
#include "math/transform.h"
#include "renderer/shader.h"

typedef struct lise_model
{
	lise_transform transform;

	lise_shader* shader;

	uint32_t mesh_count;
	lise_mesh* meshes;
} lise_model;

bool lise_model_load(lise_device* device, const char* path, lise_shader* shader, lise_model* out_model);

void lise_model_free(VkDevice device, lise_model* model);

void lise_model_draw(lise_model* model, VkDevice device, VkCommandBuffer command_buffer, uint32_t current_image);
