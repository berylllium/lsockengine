#include "loader/obj_loader.h"

#include <stdlib.h>
#include <string.h>

#include <container/darray.h>
#include <container/hashmap.h>

#include "loader/obj_format_loader.h"
#include "core/logger.h"

bool lise_obj_load(const char* path, lise_obj* out_obj)
{
	// Clear the out object.
	memset(out_obj, 0, sizeof(lise_obj));

	// Load the obj file and parse.
	lise_obj_format loaded_obj;

	if (!lise_obj_format_load(path, &loaded_obj))
	{
		LERROR("Failed to load obj file `%s`.", path);
		
		return false;
	}

	// Get directory.
	const void* path_base = path;
	const void* path_last_slash = strrchr(path, '/') + 1;

	char* directory;
	if (path_last_slash)
	{
		uint64_t directory_len = path_last_slash - path_base;
		
		directory = malloc(directory_len + 1); // + 1 for the null-terminator.

		memcpy(directory, path, directory_len);
		directory[directory_len] = '\0';
	}
	else
	{
		// There was no slash found. This means that the obj is already in the root directory.
		directory = malloc(1);
		directory[0] = '\0';
	}

	// Load materials.
	uint64_t found_line_count;
	lise_obj_format_line** found_lines;

	lise_obj_format_get_line(&loaded_obj, "mtllib", NULL, NULL, &found_line_count, &found_lines);

	if (found_line_count)
	{
		char* mtl_path = malloc(strlen(directory) + strlen(found_lines[0]->tokens[0]) + 1); // + 1 for null-term.
		memcpy(mtl_path, directory, strlen(directory) + 1); // + 1 for null-term.
		strcat(mtl_path, found_lines[0]->tokens[0]);

		lise_obj_format loaded_mtl;

		if (!lise_obj_format_load(mtl_path, &loaded_mtl))
		{
			LERROR("Failed to load MTL file `%s` for obj file `%s`.", found_lines[0]->tokens[0]);
			free(found_lines);
			free(mtl_path);
			lise_obj_free(out_obj);
			return false;
		}

		// Material count.
		uint64_t mfound_line_count;
		lise_obj_format_line** mfound_lines;

		lise_obj_format_get_line(&loaded_mtl, "newmtl", NULL, NULL, &mfound_line_count, &mfound_lines);

		out_obj->material_count = mfound_line_count;
		// Use calloc here so any eventual free on failure doesn't try to free a random memory address.
		out_obj->materials = calloc(mfound_line_count, sizeof(lise_obj_material));

		// Copy over the material names.
		for (uint64_t i = 0; i < out_obj->material_count; i++)
		{
			out_obj->materials[i].name = strdup(mfound_lines[i]->tokens[0]);
		}

		free(mfound_lines);

		// Parse remaining material data.
		for (uint64_t i = 0; i < out_obj->material_count; i++)
		{
			// Ka.
			lise_obj_format_get_line(
				&loaded_mtl, 
				"Ka",
				"newmtl",
				out_obj->materials[i].name,
				&mfound_line_count,
				&mfound_lines
			);

			if (mfound_line_count)
			{
				out_obj->materials[i].Ka.r = strtof(mfound_lines[0]->tokens[0], NULL);
				out_obj->materials[i].Ka.g = strtof(mfound_lines[0]->tokens[1], NULL);
				out_obj->materials[i].Ka.b = strtof(mfound_lines[0]->tokens[2], NULL);
			}

			free(mfound_lines);

			// Kd.
			lise_obj_format_get_line(
				&loaded_mtl, 
				"Kd",
				"newmtl",
				out_obj->materials[i].name,
				&mfound_line_count,
				&mfound_lines
			);

			if (mfound_line_count)
			{
				out_obj->materials[i].Kd.r = strtof(mfound_lines[0]->tokens[0], NULL);
				out_obj->materials[i].Kd.g = strtof(mfound_lines[0]->tokens[1], NULL);
				out_obj->materials[i].Kd.b = strtof(mfound_lines[0]->tokens[2], NULL);
			}

			free(mfound_lines);

			// Ks.
			lise_obj_format_get_line(
				&loaded_mtl, 
				"Ks",
				"newmtl",
				out_obj->materials[i].name,
				&mfound_line_count,
				&mfound_lines
			);

			if (mfound_line_count)
			{
				out_obj->materials[i].Ks.r = strtof(mfound_lines[0]->tokens[0], NULL);
				out_obj->materials[i].Ks.g = strtof(mfound_lines[0]->tokens[1], NULL);
				out_obj->materials[i].Ks.b = strtof(mfound_lines[0]->tokens[2], NULL);
			}

			free(mfound_lines);

			// Ns.
			lise_obj_format_get_line(
				&loaded_mtl, 
				"Ns",
				"newmtl",
				out_obj->materials[i].name,
				&mfound_line_count,
				&mfound_lines
			);

			if (mfound_line_count)
			{
				out_obj->materials[i].Ns = strtof(mfound_lines[0]->tokens[0], NULL);
			}

			free(mfound_lines);

			// Ni.
			lise_obj_format_get_line(
				&loaded_mtl, 
				"Ni",
				"newmtl",
				out_obj->materials[i].name,
				&mfound_line_count,
				&mfound_lines
			);

			if (mfound_line_count)
			{
				out_obj->materials[i].Ni = strtof(mfound_lines[0]->tokens[0], NULL);
			}

			free(mfound_lines);

			// d.
			lise_obj_format_get_line(
				&loaded_mtl, 
				"d",
				"newmtl",
				out_obj->materials[i].name,
				&mfound_line_count,
				&mfound_lines
			);

			if (mfound_line_count)
			{
				out_obj->materials[i].d = strtof(mfound_lines[0]->tokens[0], NULL);
			}

			free(mfound_lines);

			// illum.
			lise_obj_format_get_line(
				&loaded_mtl, 
				"illum",
				"newmtl",
				out_obj->materials[i].name,
				&mfound_line_count,
				&mfound_lines
			);

			if (mfound_line_count)
			{
				out_obj->materials[i].illum = strtol(mfound_lines[0]->tokens[0], NULL, 10);
			}

			free(mfound_lines);

			// map_Ka.
			lise_obj_format_get_line(
				&loaded_mtl, 
				"map_Ka",
				"newmtl",
				out_obj->materials[i].name,
				&mfound_line_count,
				&mfound_lines
			);

			if (mfound_line_count)
			{
				out_obj->materials[i].map_Ka = strdup(mfound_lines[0]->tokens[0]);
			}

			free(mfound_lines);

			// map_Kd.
			lise_obj_format_get_line(
				&loaded_mtl, 
				"map_Kd",
				"newmtl",
				out_obj->materials[i].name,
				&mfound_line_count,
				&mfound_lines
			);

			if (mfound_line_count)
			{
				out_obj->materials[i].map_Kd = strdup(mfound_lines[0]->tokens[0]);
			}

			free(mfound_lines);

			// map_Ks.
			lise_obj_format_get_line(
				&loaded_mtl, 
				"map_Ks",
				"newmtl",
				out_obj->materials[i].name,
				&mfound_line_count,
				&mfound_lines
			);

			if (mfound_line_count)
			{
				out_obj->materials[i].map_Ks = strdup(mfound_lines[0]->tokens[0]);
			}

			free(mfound_lines);
			
			// map_Ns.
			lise_obj_format_get_line(
				&loaded_mtl, 
				"map_Ns",
				"newmtl",
				out_obj->materials[i].name,
				&mfound_line_count,
				&mfound_lines
			);

			if (mfound_line_count)
			{
				out_obj->materials[i].map_Ns = strdup(mfound_lines[0]->tokens[0]);
			}

			free(mfound_lines);

			// map_d.
			lise_obj_format_get_line(
				&loaded_mtl, 
				"map_d",
				"newmtl",
				out_obj->materials[i].name,
				&mfound_line_count,
				&mfound_lines
			);

			if (mfound_line_count)
			{
				out_obj->materials[i].map_d = strdup(mfound_lines[0]->tokens[0]);
			}

			free(mfound_lines);

			// map_bump.
			lise_obj_format_get_line(
				&loaded_mtl, 
				"map_bump",
				"newmtl",
				out_obj->materials[i].name,
				&mfound_line_count,
				&mfound_lines
			);

			if (mfound_line_count)
			{
				out_obj->materials[i].map_bump = strdup(mfound_lines[0]->tokens[0]);
			}

			free(mfound_lines);
		}

		lise_obj_format_free(&loaded_mtl);
	}
	else
	{
		out_obj->material_count = 0;
		out_obj->materials = NULL;
	}

	// Load meshes.
	lise_obj_format_get_line(&loaded_obj, "o", NULL, NULL, &found_line_count, &found_lines);

	if (found_line_count)
	{
		// Meshes are present. Allocate meshes array.
		out_obj->mesh_count = found_line_count;
		out_obj->meshes = malloc(found_line_count * sizeof(lise_obj_mesh));

		// Copy over mesh names.
		for (uint32_t i = 0; i < out_obj->mesh_count; i++)
		{
			out_obj->meshes[i].name = strdup(found_lines[i]->tokens[0]);
		}

		free(found_lines);

		// Parse remaining mesh data.
		for (uint32_t i = 0; i < out_obj->mesh_count; i++)
		{
			// Position.
			lise_obj_format_get_line(
				&loaded_obj,
				"v",
				"o",
				out_obj->meshes[i].name,
				&found_line_count,
				&found_lines
			);

			uint32_t position_count = found_line_count;
			lise_vec3* positions = malloc(position_count * sizeof(lise_vec3));

			for (uint64_t j = 0; j < found_line_count; j++)
			{
				positions[j].x = strtof(found_lines[j]->tokens[0], NULL);
				positions[j].y = strtof(found_lines[j]->tokens[1], NULL);
				positions[j].z = strtof(found_lines[j]->tokens[2], NULL);
			}

			free(found_lines);

			// Texture coordinate.
			lise_obj_format_get_line(
				&loaded_obj,
				"vt",
				"o",
				out_obj->meshes[i].name,
				&found_line_count,
				&found_lines
			);

			uint32_t texture_coordinate_count = found_line_count;
			lise_vec2* texture_coordinates = malloc(texture_coordinate_count * sizeof(lise_vec2));

			for (uint64_t j = 0; j < found_line_count; j++)
			{
				texture_coordinates[j].u = strtof(found_lines[j]->tokens[0], NULL);
				texture_coordinates[j].v = strtof(found_lines[j]->tokens[1], NULL);
			}

			free(found_lines);

			// Normal coordinate.
			lise_obj_format_get_line(
				&loaded_obj,
				"vn",
				"o",
				out_obj->meshes[i].name,
				&found_line_count,
				&found_lines
			);

			uint32_t normal_count = found_line_count;
			lise_vec3* normals = malloc(normal_count * sizeof(lise_vec3));

			for (uint64_t j = 0; j < found_line_count; j++)
			{
				normals[j].x = strtof(found_lines[j]->tokens[0], NULL);
				normals[j].y = strtof(found_lines[j]->tokens[1], NULL);
				normals[j].z = strtof(found_lines[j]->tokens[2], NULL);
			}

			free(found_lines);

			// Faces.
			lise_obj_format_get_line(
				&loaded_obj,
				"f",
				"o",
				out_obj->meshes[i].name,
				&found_line_count,
				&found_lines
			);

			// Stores the unique vertices.
			blib_darray unique_vertices = blib_darray_create(lise_vertex);

			// Stores the indices to the unique vertices. Use the "value" pointer as an integer. Indices start from
			// one (1) in this hashmap. As a pointer of 0 is a null-pointer and would mess up the hashmap.
			blib_hashmap vertex_indices = blib_hashmap_create();

			blib_darray indices = blib_darray_create(uint32_t, found_line_count * 3);

			for (uint64_t j = 0; j < found_line_count; j++)
			{
				for (uint64_t k = 0; k < 3; k++)
				{
					char* vertex_str = found_lines[j]->tokens[k];

					uint64_t found_index = (uint64_t) blib_hashmap_get(&vertex_indices, vertex_str);

					if (found_index)
					{
						// This vertex has already been stored in the unique vertices array. Only add the index to the
						// indices array.
						uint32_t idx = found_index - 1; // Subtract one because the hashmap stores indices starting from
														// 1.

						blib_darray_push_back(&indices, &idx);
					}
					else
					{
						// Unique vertex has not been stored yet.
						// Store index in hashmap.
						blib_hashmap_set(&vertex_indices, vertex_str, (void*) unique_vertices.size + 1);

						// Push back index of vertex.
						uint32_t idx = unique_vertices.size;
						blib_darray_push_back(&indices, &idx);

						// Parse vertex.
						lise_vertex vertex;

						char* pos_index = strtok(vertex_str, "/");
						char* tex_cord_index = strtok(NULL, "/");
						char* norm_index = strtok(NULL, "/");

						// Subtract one from the indices because the OBJ format starts counting from one (1).
						vertex.position = positions[strtol(pos_index, NULL, 10) - 1];
						vertex.tex_coord = texture_coordinates[strtol(tex_cord_index, NULL, 10) - 1];
						vertex.normal = normals[strtol(norm_index, NULL, 10) - 1];
						
						blib_darray_push_back(&unique_vertices, &vertex);
					}
				}
			}

			out_obj->meshes[i].vertex_count = unique_vertices.size;
			out_obj->meshes[i].vertices = malloc(unique_vertices.size * sizeof(lise_vertex));
			memcpy(out_obj->meshes[i].vertices, unique_vertices.data, unique_vertices.size * sizeof(lise_vertex));

			out_obj->meshes[i].index_count = indices.size;
			out_obj->meshes[i].indices = malloc(indices.size * sizeof(uint32_t));
			memcpy(out_obj->meshes[i].indices, indices.data, indices.size * sizeof(uint32_t));

			blib_darray_free(&unique_vertices);
			blib_darray_free(&indices);
			free(normals);
			free(texture_coordinates);
			free(positions);
			free(found_lines);

			// Material.
			lise_obj_format_get_line(
				&loaded_obj,
				"usemtl",
				"o",
				out_obj->meshes[i].name,
				&found_line_count,
				&found_lines
			);

			if (found_line_count)
			{
				// Uses material
				// Find material pointer.
				for (uint32_t j = 0; j < out_obj->material_count; j++)
				{
					if (strcmp(found_lines[0]->tokens[0], out_obj->materials[j].name) == 0)
					{
						out_obj->meshes[i].material = &out_obj->materials[j];
						break;
					}
				}
			}

			free(found_lines);
		}
	}
	else
	{
		out_obj->mesh_count = 0;
		out_obj->meshes = NULL;
	}

	// Free.
	lise_obj_format_free(&loaded_obj);

	free(directory);

	return true;
}

void lise_obj_free(lise_obj* obj)
{
	// Free materials.
	for (uint32_t i = 0; i < obj->material_count; i++)
	{
		free(obj->materials[i].name);
		free(obj->materials[i].map_Ka);
		free(obj->materials[i].map_Kd);
		free(obj->materials[i].map_Ks);
		free(obj->materials[i].map_Ns);
		free(obj->materials[i].map_d);
		free(obj->materials[i].map_bump);
	}

	free(obj->materials);
}
