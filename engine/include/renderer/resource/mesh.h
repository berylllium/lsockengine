#pragma once

#include "definitions.h"
#include "math/vertex.h"
#include "renderer/resource/texture.h"
#include "renderer/vulkan_buffer.h"

typedef struct lise_mesh
{
	uint64_t* vertex_count;
	lise_vertex* vertices;

	uint64_t index_count;
	lise_vec3 indices;

	lise_texture* diffuse_texture;

	lise_vulkan_buffer vertex_buffer;
	lise_vulkan_buffer index_buffer;
} lise_mesh;
