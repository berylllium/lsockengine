#include "math/transform.h"

lise_transform lise_transform_create()
{
	lise_transform transform = {};

	transform.parent = NULL;
	transform.children = blib_darray_create(lise_transform*);
	
	transform.scale = LVEC3_ONE;
	transform.rotation = LVEC3_ZERO;
	transform.position = LVEC3_ZERO;

	lise_transform_recalculate_transformation_matrix(&transform);

	return transform;
}

void lise_transform_free(lise_transform* transform)
{
	// Remove as parent.
	for (uint64_t i = 0; i < transform->children.size; i++)
	{
		lise_transform* child = *((lise_transform**) blib_darray_get(&transform->children, i));
		child->parent = NULL;
	}

	blib_darray_free(&transform->children);
}

void lise_transform_recalculate_transformation_matrix(lise_transform* transform)
{
	transform->transformation_matrix = LMAT4X4_IDENTITY;

	// Scale.
	transform->transformation_matrix = lise_mat4x4_mul(
		transform->transformation_matrix,
		lise_mat4x4_scale(transform->scale)
	);

	// Rotate.
	transform->transformation_matrix = lise_mat4x4_mul(
		transform->transformation_matrix,
		lise_mat4x4_euler_xyz(transform->rotation.x, transform->rotation.y, transform->rotation.z)
	);

	// Translate.
	transform->transformation_matrix = lise_mat4x4_mul(
		transform->transformation_matrix,
		lise_mat4x4_translation(transform->position)
	);
	
	// Parent transform.
	if (transform->parent)
	{
		transform->transformation_matrix =  lise_mat4x4_mul(
			transform->transformation_matrix,
			transform->parent->transformation_matrix
		);
	}
}

void lise_transform_update(lise_transform* transform)
{
	// Recalculate transfomration matrix.
	lise_transform_recalculate_transformation_matrix(transform);

	// Update children.
	for (uint64_t i = 0; i < transform->children.size; i++)
	{
		lise_transform* child = *((lise_transform**) blib_darray_get(&transform->children, i));
		lise_transform_update(child);
	}
}

void lise_transform_add_child(lise_transform* parent, lise_transform* child)
{
	child->parent = parent;

	blib_darray_push_back(&parent->children, &child);
}
