/**
 * @file obj_loader.hpp
 * @brief This header file contains definitions of structures and functions relating to loading obj files.
 * 
 * The term "obj file" should not be confused with the term "obj format file", as the former refers to exclusively
 * <a href="https://en.wikipedia.org/wiki/Wavefront_.obj_file" target="_blank">Wavefront obj files</a>, and thus to only
 * geometry data. The latter can refer to both geometry data and any other generic data, such as shader configurations.
 */
#pragma once

#include <optional>
#include <string>
#include <vector>

#include "definitions.hpp"
#include "math/vector3.hpp"
#include "math/vertex.hpp"

namespace lise
{

/**
 * @brief A structure that contains all possible material attributes as specified by the 
 * <a href="https://en.wikipedia.org/wiki/Wavefront_.obj_file" target="_blank">Wavefront obj format</a>.
 * 
 * All the strings stored within this structure are owned by the structure itself.
 */
struct ObjMaterial
{
	/**
	 * @brief Material name.
	 */
	std::string name;

	/**
	 * @brief Ambient color.
	 */
	vector3f Ka;

	/**
	 * @brief Diffuse color.
	 */
	vector3f Kd;

	/**
	 * @brief Specular color.
	 */
	vector3f Ks;

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
	std::string map_Ka;

	/**
	 * @brief Diffuse texture map.
	 */
	std::string map_Kd;

	/**
	 * @brief Specular texture map.
	 */
	std::string map_Ks;

	/**
	 * @brief Specular highlight map.
	 */
	std::string map_Ns;

	/**
	 * @brief Alpha texture map.
	 */
	std::string map_d;

	/**
	 * @brief Bump map.
	 */
	std::string map_bump;
};

/**
 * @brief A structure that stores mesh data.
 * 
 * A "mesh" is a collection of unique vertices and indices which reference those vertices. Each mesh can only have
 * one material.
 */
struct ObjMesh
{
	/**
	 * @brief The name of the mesh, as specified in the obj file.
	 * 
	 * This string is owned by the \ref lise_obj_mesh object.
	 */
	std::string name;

	/**
	 * @brief An array of vertices.
	 * 
	 * The allocation is owned by the \ref lise_obj_mesh object.
	 */
	std::vector<vertex> vertices;

	/**
	 * @brief An array of indices.
	 * 
	 * The allocation is owned by the \ref lise_obj_mesh object.
	 */
	std::vector<uint32_t> indices;

	/**
	 * @brief A pointer to a loaded \ref lise_obj_material in the \ref lise_obj struct.
	 * 
	 * The allocation is not owned by the \ref lise_obj_mesh object, but by the \ref lise_obj object.
	 */
	ObjMaterial* material;
};

/**
 * @brief A structure that contains parsed meshes and materials from an obj file.
 */
struct Obj
{
	/**
	 * @brief Tries to load and parse the obj file.
	 * 
	 * @param path The path to the obj file. Can be relative or absolute.
	 * @return The loaded obj.
	 */
	static std::optional<Obj> load(const std::string& path);
	
	/**
	 * @brief An array of meshes.
	 */
	std::vector<ObjMesh> meshes;

	/**
	 * @brief An array of materials.
	 */
	std::vector<ObjMaterial> materials;
};

}
