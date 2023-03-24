#pragma once

#include <container/darray.h>

#include "definitions.h"
#include "mat4x4.h"

typedef struct lise_transform
{
	struct lise_transform* parent;
	blib_darray children; // lise_transform*
	
	lise_vec3 scale;
	lise_vec3 rotation;
	lise_vec3 position;

	lise_mat4x4 transformation_matrix;
} lise_transform;

lise_transform lise_transform_create();

void lise_transform_free(lise_transform* transform);

void lise_transform_recalculate_transformation_matrix(lise_transform* transform);

void lise_transform_update(lise_transform* transform);

void lise_transform_add_child(lise_transform* parent, lise_transform* child);
