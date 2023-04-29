#pragma once

#include <container/darray.h>

#include "definitions.h"
#include "math/transform.h"

#include "renderer/resource/mesh.h"

typedef enum lise_node_type
{
	LISE_NODE_TYPE_INVALID,
	LISE_NODE_TYPE_NODE,
	LISE_NODE_TYPE_SPATIAL,
	LISE_NODE_TYPE_MESH_RENDERER
} lise_node_type;

typedef struct lise_node_abstract
{
	lise_node_type node_type;
	void* node;
} lise_node_abstract;

typedef struct lise_node
{
	char* name;

	lise_node_abstract parent_note;

	blib_darray children; // lise_node_abstract
} lise_node;

typedef struct lise_node_spatial
{
	lise_node node;

	lise_transform transform;
} lise_node_spatial;

typedef struct lise_node_mesh_renderer
{
	lise_node_spatial spatial;

	lise_mesh mesh;
} lise_node_mesh_renderer;
