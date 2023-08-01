#include "loader/obj_loader.hpp"

#include <cstdlib>
#include <unordered_map>

#include <simple-logger.hpp>

#include "loader/obj_format_loader.hpp"
#include "util/string_utils.hpp"

namespace lise
{

std::optional<Obj> Obj::load(const std::string& path)
{
	Obj out_obj;
	// Load the obj file and parse.
	ObjFormat loaded_obj_format;

	if (!obj_format_load(path, loaded_obj_format))
	{
		sl::log_error("Failed to load obj file `{}`.", path);
		
		return {};
	}

	// Get directory.
	auto path_last_slash_pos = path.rfind("/");

	std::string directory;
	if (path_last_slash_pos != std::string::npos)
	{
		directory = path.substr(0, path_last_slash_pos + 1);
	}

	// Load materials.
	auto found_lines = obj_format_get_line(loaded_obj_format, "mtllib"); 

	if (found_lines.size())
	{
		auto mtl_path = directory + found_lines[0]->tokens[0];

		ObjFormat loaded_mtl;

		if (!obj_format_load(mtl_path, loaded_mtl))
		{
			sl::log_error("Failed to load MTL file `{}` for obj file `{}`.", mtl_path, path);
			
			return {};
		}

		// Material count.
		auto mfound_lines = obj_format_get_line(loaded_mtl, "newmtl");

		out_obj.materials.resize(mfound_lines.size());

		// Copy over the material names.
		for (uint64_t i = 0; i < out_obj.materials.size(); i++)
		{
			out_obj.materials[i].name = mfound_lines[i]->tokens[0];
		}

		// Parse remaining material data.
		for (uint64_t i = 0; i < out_obj.materials.size(); i++)
		{
			// Ka.
			mfound_lines = obj_format_get_line(
				loaded_mtl, 
				"Ka",
				"newmtl",
				out_obj.materials[i].name
			);

			if (mfound_lines.size() > 0)
			{
				out_obj.materials[i].Ka.r = strtof(mfound_lines[0]->tokens[0].c_str(), NULL);
				out_obj.materials[i].Ka.g = strtof(mfound_lines[0]->tokens[1].c_str(), NULL);
				out_obj.materials[i].Ka.b = strtof(mfound_lines[0]->tokens[2].c_str(), NULL);
			}

			// Kd.
			mfound_lines = obj_format_get_line(
				loaded_mtl, 
				"Kd",
				"newmtl",
				out_obj.materials[i].name
			);

			if (mfound_lines.size())
			{
				out_obj.materials[i].Kd.r = strtof(mfound_lines[0]->tokens[0].c_str(), NULL);
				out_obj.materials[i].Kd.g = strtof(mfound_lines[0]->tokens[1].c_str(), NULL);
				out_obj.materials[i].Kd.b = strtof(mfound_lines[0]->tokens[2].c_str(), NULL);
			}

			// Ks.
			mfound_lines = obj_format_get_line(
				loaded_mtl, 
				"Ks",
				"newmtl",
				out_obj.materials[i].name
			);

			if (mfound_lines.size())
			{
				out_obj.materials[i].Ks.r = strtof(mfound_lines[0]->tokens[0].c_str(), NULL);
				out_obj.materials[i].Ks.g = strtof(mfound_lines[0]->tokens[1].c_str(), NULL);
				out_obj.materials[i].Ks.b = strtof(mfound_lines[0]->tokens[2].c_str(), NULL);
			}

			// Ns.
			mfound_lines = obj_format_get_line(
				loaded_mtl, 
				"Ns",
				"newmtl",
				out_obj.materials[i].name
			);

			if (mfound_lines.size())
			{
				out_obj.materials[i].Ns = strtof(mfound_lines[0]->tokens[0].c_str(), NULL);
			}

			// Ni.
			mfound_lines = obj_format_get_line(
				loaded_mtl, 
				"Ni",
				"newmtl",
				out_obj.materials[i].name
			);

			if (mfound_lines.size())
			{
				out_obj.materials[i].Ni = strtof(mfound_lines[0]->tokens[0].c_str(), NULL);
			}

			// d.
			mfound_lines = obj_format_get_line(
				loaded_mtl, 
				"d",
				"newmtl",
				out_obj.materials[i].name
			);

			if (mfound_lines.size())
			{
				out_obj.materials[i].d = strtof(mfound_lines[0]->tokens[0].c_str(), NULL);
			}

			// illum.
			mfound_lines = obj_format_get_line(
				loaded_mtl, 
				"illum",
				"newmtl",
				out_obj.materials[i].name
			);

			if (mfound_lines.size())
			{
				out_obj.materials[i].illum = strtol(mfound_lines[0]->tokens[0].c_str(), NULL, 10);
			}

			// map_Ka.
			mfound_lines = obj_format_get_line(
				loaded_mtl, 
				"map_Ka",
				"newmtl",
				out_obj.materials[i].name
			);

			if (mfound_lines.size())
			{
				out_obj.materials[i].map_Ka = mfound_lines[0]->tokens[0];
			}

			// map_Kd.
			mfound_lines = obj_format_get_line(
				loaded_mtl, 
				"map_Kd",
				"newmtl",
				out_obj.materials[i].name
			);

			if (mfound_lines.size())
			{
				out_obj.materials[i].map_Kd = mfound_lines[0]->tokens[0];
			}

			// map_Ks.
			mfound_lines = obj_format_get_line(
				loaded_mtl, 
				"map_Ks",
				"newmtl",
				out_obj.materials[i].name
			);

			if (mfound_lines.size())
			{
				out_obj.materials[i].map_Ks = mfound_lines[0]->tokens[0];
			}

			// map_Ns.
			mfound_lines = obj_format_get_line(
				loaded_mtl, 
				"map_Ns",
				"newmtl",
				out_obj.materials[i].name
			);

			if (mfound_lines.size())
			{
				out_obj.materials[i].map_Ns = mfound_lines[0]->tokens[0];
			}

			// map_d.
			mfound_lines = obj_format_get_line(
				loaded_mtl, 
				"map_d",
				"newmtl",
				out_obj.materials[i].name
			);

			if (mfound_lines.size())
			{
				out_obj.materials[i].map_d = mfound_lines[0]->tokens[0];
			}

			// map_bump.
			mfound_lines = obj_format_get_line(
				loaded_mtl, 
				"map_bump",
				"newmtl",
				out_obj.materials[i].name
			);

			if (mfound_lines.size())
			{
				out_obj.materials[i].map_bump = mfound_lines[0]->tokens[0];
			}
		}
	}

	// Load meshes.
	found_lines = obj_format_get_line(loaded_obj_format, "o");

	if (found_lines.size())
	{
		// Meshes are present. Allocate meshes array.
		out_obj.meshes.resize(found_lines.size());

		// Copy over mesh names.
		for (uint32_t i = 0; i < out_obj.meshes.size(); i++)
		{
			out_obj.meshes[i].name = found_lines[i]->tokens[0];
		}

		// Position.
		found_lines = obj_format_get_line(loaded_obj_format, "v");

		std::vector<vector3f> positions(found_lines.size());

		for (uint64_t j = 0; j < found_lines.size(); j++)
		{
			positions[j].x = strtof(found_lines[j]->tokens[0].c_str(), NULL);
			positions[j].y = strtof(found_lines[j]->tokens[1].c_str(), NULL);
			positions[j].z = strtof(found_lines[j]->tokens[2].c_str(), NULL);
		}

		// Texture coordinate.
		found_lines = obj_format_get_line(loaded_obj_format, "vt");

		std::vector<vector2f> texture_coordinates(found_lines.size());

		for (uint64_t j = 0; j < found_lines.size(); j++)
		{
			texture_coordinates[j].u = strtof(found_lines[j]->tokens[0].c_str(), NULL);
			texture_coordinates[j].v = strtof(found_lines[j]->tokens[1].c_str(), NULL);
		}

		// Normal coordinate.
		found_lines = obj_format_get_line(loaded_obj_format, "vn");

		std::vector<vector3f> normals(found_lines.size());

		for (uint64_t j = 0; j < found_lines.size(); j++)
		{
			normals[j].x = strtof(found_lines[j]->tokens[0].c_str(), NULL);
			normals[j].y = strtof(found_lines[j]->tokens[1].c_str(), NULL);
			normals[j].z = strtof(found_lines[j]->tokens[2].c_str(), NULL);
		}

		// Parse remaining mesh data.
		for (uint32_t i = 0; i < out_obj.meshes.size(); i++)
		{
			// Faces.
			found_lines = obj_format_get_line(loaded_obj_format, "f", "o", out_obj.meshes[i].name);

			// Stores the unique vertices.
			std::vector<vertex> unique_vertices;

			// Stores the indices to the unique vertices.
			std::unordered_map<std::string, uint32_t> vertex_indices;

			std::vector<uint32_t> indices;
			indices.reserve(found_lines.size() * 3);

			for (uint64_t j = 0; j < found_lines.size(); j++)
			{
				for (uint64_t k = 0; k < 3; k++)
				{
					const std::string& vertex_str = found_lines[j]->tokens[k];

					if (vertex_indices.contains(vertex_str))
					{
						uint32_t found_index = vertex_indices[vertex_str];

						// This vertex has already been stored in the unique vertices array.
						// Only add the index to the indices array.
						indices.push_back(found_index);
					}
					else
					{
						// Unique vertex has not been stored yet.
						// Store index in hashmap.
						vertex_indices[vertex_str] = unique_vertices.size();

						// Push back index of vertex.
						indices.push_back(unique_vertices.size());

						// Parse vertex.
						vertex vertex;

						std::vector<std::string> vindices = split(vertex_str, "/");

						// Subtract one from the indices because the OBJ format starts counting from one (1).
						vertex.position = positions[strtol(vindices[0].c_str(), NULL, 10) - 1];
						vertex.tex_coord = texture_coordinates[strtol(vindices[1].c_str(), NULL, 10) - 1];
						vertex.normal = normals[strtol(vindices[2].c_str(), NULL, 10) - 1];
						
						unique_vertices.push_back(vertex);
					}
				}
			}

			out_obj.meshes[i].vertices = unique_vertices;

			out_obj.meshes[i].indices = indices;

			// Material.
			found_lines = obj_format_get_line(
				loaded_obj_format,
				"usemtl",
				"o",
				out_obj.meshes[i].name
			);

			if (found_lines.size())
			{
				// Uses material
				// Find material pointer.
				for (ObjMaterial& mat : out_obj.materials)
				{
					if (found_lines[0]->tokens[0] == mat.name)
					{
						out_obj.meshes[i].material = &mat;
						break;
					}
				}
			}
		}
	}

	return out_obj;
}

}
