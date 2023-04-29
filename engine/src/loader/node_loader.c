#include "loader/node_loader.h"

#include <stdlib.h>
#include <string.h>

#include "core/logger.h"

#include "loader/obj_format_loader.h"

// Static helper functions
static bool parse_spatial(lise_obj_format loaded_format, char* name, lise_node_spatial* node);
static lise_node_type text_to_node_type(const char* t);

bool lise_node_loader_load(const char* path, lise_node_abstract* out_node)
{
	// Zero out the out node.
	memset(out_node, 0, sizeof(lise_node));

	// Load and parse the obj file format.
	lise_obj_format loaded_format;

	if (!lise_obj_format_load(path, &loaded_format))
	{
		LERROR("Failed to load node file `%s`.", path);
		
		return false;
	}

	// Get all the nodes.
	uint64_t node_count;
	lise_obj_format_line** nodes;

	lise_obj_format_get_line(&loaded_format, "n", NULL, NULL, &node_count, &nodes);
	
	if (!node_count)
	{
		LERROR("Node file `%s` contains no nodes. A node file must contain at least one node.", path);

		return false;
	}

	// Iterate and parse nodes.
	for (uint64_t i = 0; i < node_count; i++)
	{
		if (nodes[i]->token_count == 1)
		{
			// Node is an import node.

		}
		else if (nodes[i]->token_count == 2)
		{
			// Node is a direct node.

			// Get node type.
			lise_node_type node_type = text_to_node_type(nodes[i]->tokens[1]);

			if (node_type == -1)
			{
				LERROR("Error while parsing node `%s`: `%s` is not a valid node type.", path, nodes[i]->tokens[1]);

				free(nodes);
				return false;
			}

			// Allocate node.
			void* node;

			switch (node_type)
			{
			case LISE_NODE_TYPE_NODE:
				node = calloc(1, sizeof(lise_node));
				break;
			case LISE_NODE_TYPE_SPATIAL:
				node = calloc(1, sizeof(lise_node_spatial));
				break;
			case LISE_NODE_TYPE_MESH_RENDERER:
				node = calloc(1, sizeof(lise_node_mesh_renderer));
				break;
			case LISE_NODE_TYPE_INVALID:
			default:
				LERROR("Error while parsing node `%s`: Node type could not be resolved.", path);

				free(nodes);
				return false;
			}

			// Get parent.
			uint64_t found_line_count;
			lise_obj_format_line** found_lines;

			lise_obj_format_get_line(&loaded_format, "p", "n", nodes[i]->tokens[0], &found_line_count, &found_lines);

			if (i == 0)
			{
				// Node is root node.
				out_node->node_type = node_type;
				out_node->node = node;
			}
			else
			{
				// Node is not root node.
				if (found_line_count == 0)
				{
					// Not first node in tree. Node is not root node. Node is not allowed to have no parent.
					LERROR(
						"Error while parsing node `%s`. Node `%s` is not a root node; parent must be declared.",
						path,
						nodes[i]->tokens[0]
					);

					free(nodes);
					return false;
				}
			}
		}
		else
		{
			LERROR("Error while parsing node `%s` Node must contain a name and type OR a file path.", path);

			free(nodes);
			return false;
		}





	}
}

void lise_node_loader_free(lise_node_abstract* node)
{

}

// Static helper functions

static bool parse_spatial(lise_obj_format loaded_format, char* name, lise_node_spatial* node)
{
	// Get model path.
	uint64_t found_line_count;
	lise_obj_format_line** found_lines;

	// Position.
	lise_obj_format_get_line(&loaded_format, "tp", "n", name, &found_line_count, &found_lines);

	if (found_line_count)
	{
		if (found_lines[0]->token_count != 3)
		{
			LERROR(
				"Error while parsing spatial node `%s`: Spatial node attribute `tp` must have exactly 3 arguments.",
				name
			);

			free(found_lines);
			return false;
		}

		// Parse values.
		node->transform.position.x = strtof(found_lines[0]->tokens[0], NULL);
		node->transform.position.y = strtof(found_lines[0]->tokens[1], NULL);
		node->transform.position.z = strtof(found_lines[0]->tokens[2], NULL);
	}
	else
	{
		// Use default values.
		node->transform.position.x = 0.0f;
		node->transform.position.y = 0.0f;
		node->transform.position.z = 0.0f;
	}

	// TODO: Parse rotation and scale.

	// Set transform parent.
	

	return true;
}

static lise_node_type text_to_node_type(const char* t)
{
	if (strcmp(t, "node") == 0)
	{
		return LISE_NODE_TYPE_NODE;
	}
	else if (strcmp(t, "spatial") == 0)
	{
		return LISE_NODE_TYPE_SPATIAL;
	}
	else if (strcmp(t, "mesh_renderer") == 0)
	{

	}
	else
	{
		return -1;
	}
}
