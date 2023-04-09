/**
 * @file obj_loader.h
 * @brief This header file contains definitions of structures and functions relating to loading obj files.
 * 
 * The term "obj file" should not be confused with the term "obj format file", as the former refers to exclusively
 * <a href="https://en.wikipedia.org/wiki/Wavefront_.obj_file" target="_blank">Wavefront obj files</a>, and thus to only
 * geometry data. The latter can refer to both geometry data and any other generic data, such as shader configurations.
 */
#pragma once

#include "definitions.h"
#include "math/vector3.h"
#include "math/vertex.h"

/**
 * @brief A structure that contains all possible material attributes as specified by the 
 * <a href="https://en.wikipedia.org/wiki/Wavefront_.obj_file" target="_blank">Wavefront obj format</a>.
 * 
 * All the strings stored within this structure are owned by the structure itself.
 */
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

/**
 * @brief A structure that stores mesh data.
 * 
 * A "mesh" is a collection of unique vertices and indices which reference those vertices. Each mesh can only have
 * one material.
 */
typedef struct lise_obj_mesh
{
	/**
	 * @brief The name of the mesh, as specified in the obj file.
	 * 
	 * This string is owned by the \ref lise_obj_mesh object.
	 */
	char* name;

	/**
	 * @brief The amount of vertices stored in the memory allocation pointed to by the \ref vertices member.
	 */
	uint32_t vertex_count;

	/**
	 * @brief An array of vertices.
	 * 
	 * The allocation is owned by the \ref lise_obj_mesh object.
	 */
	lise_vertex* vertices;

	/**
	 * @brief The amount of indices stored in the memory allocation pointed to by the \ref indices member.
	 */
	uint32_t index_count;
	
	/**
	 * @brief An array of indices.
	 * 
	 * The allocation is owned by the \ref lise_obj_mesh object.
	 */
	uint32_t* indices;

	/**
	 * @brief A pointer to a loaded \ref lise_obj_material in the \ref lise_obj struct.
	 * 
	 * The allocation is not owned by the \ref lise_obj_mesh object, but by the \ref lise_obj object.
	 */
	lise_obj_material* material;
} lise_obj_mesh;

/**
 * @brief A structure that contains parsed meshes and materials from an obj file.
 */
typedef struct lise_obj
{
	/**
	 * @brief The amount of meshes stored in the allocation pointed to by the \ref meshes member.
	 */
	uint32_t mesh_count;

	/**
	 * @brief An array of meshes.
	 * 
	 * The allocation is owned by the \ref lise_obj object.
	 */
	lise_obj_mesh* meshes;

	/**
	 * @brief The amount of materials stored in the allocation pointed to by the \ref materials member.
	 */
	uint32_t material_count;

	/**
	 * @brief An array of materials.
	 * 
	 * The allocation is owned by the \ref lise_obj object.
	 */
	lise_obj_material* materials;
} lise_obj;

/**
 * @brief Tries to load and parse the obj file.
 * 
 * @param path The path to the obj file. Can be relative or absolute.
 * @param out_obj [out] A pointer to where to load the \ref lise_obj to.
 * @return true if the obj file was successfully loaded and parsed.
 * @return false if there was an error loading or parsing the obj file.
 */
bool lise_obj_load(const char* path, lise_obj* out_obj);

/**
 * @brief Frees the \ref lise_obj.
 * 
 * @param obj the \ref lise_obj to free.
 */
void lise_obj_free(lise_obj* obj);
