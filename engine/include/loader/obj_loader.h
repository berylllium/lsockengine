#pragma once

#include "definitions.h"
#include "math/vector3.h"
#include "math/vertex.h"

typedef struct lise_obj_material
{
	/**
	 * @brief Material name.
	 */
	char* name;

	/**
	 * @brief Ambient color.
	 */
	lise_vec3 Ka;

	/**
	 * @brief Diffuse color.
	 */
	lise_vec3 Kd;

	/**
	 * @brief Specular color.
	 */
	lise_vec3 Ks;

	/**
	 * @brief Specular exponent.
	 */
	float Ns;

	/**
	 * @brief Optical density.
	 */
	float Ni;

	/**
	 * @brief Dissolve.
	 */
	float d;

	/**
	 * @brief Illumination.
	 */
	int illum;

	/**
	 * @brief Ambient texture map.
	 */
	char* map_Ka;

	/**
	 * @brief Diffuse texture map.
	 */
	char* map_Kd;

	/**
	 * @brief Specular texture map.
	 */
	char* map_Ks;

	/**
	 * @brief Specular highlight map.
	 */
	char* map_Ns;

	/**
	 * @brief Alpha texture map.
	 */
	char* map_d;

	/**
	 * @brief Bump map.
	 */
	char* map_bump;
} lise_obj_material;

typedef struct lise_obj_mesh
{
	/**
	 * @brief The name of the mesh, as specified in the obj file.
	 */
	char* name;

	uint32_t vertex_count;
	lise_vertex* vertices;

	uint32_t index_count;
	uint32_t* indices;

	/**
	 * @brief A pointer to a loaded material in the lise_obj struct.
	 */
	lise_obj_material* material;
} lise_obj_mesh;

typedef struct lise_obj
{
	uint32_t mesh_count;
	lise_obj_mesh* meshes;

	uint32_t material_count;
	lise_obj_material* materials;
} lise_obj;

bool lise_obj_load(const char* path, lise_obj* out_obj);

void lise_obj_free(lise_obj* obj);
