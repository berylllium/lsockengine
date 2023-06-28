#include "math/transform.hpp"

namespace lise
{

Transform::Transform() : parent(nullptr), scale(LVEC3_ONE), rotation(LVEC3_ZERO), position(LVEC3_ZERO)
{
	recalculate_transformation_matrix();
}

void Transform::set_parent(Transform* new_parent)
{
	parent = new_parent;

	recalculate_transformation_matrix();
	update_children();
}

void Transform::add_child(Transform* new_child)
{
	new_child->set_parent(this);

	children.push_back(new_child);
}

void Transform::set_scale(vector3f new_scale)
{
	scale = new_scale;

	recalculate_transformation_matrix();
	update_children();
}

void Transform::set_scale(float new_x, float new_y, float new_z)
{
	set_scale({new_x, new_y, new_z});
}

void Transform::set_rotation(vector3f new_rot)
{
	rotation = new_rot;

	recalculate_transformation_matrix();
	update_children();
}

void Transform::set_rotation(float new_x, float new_y, float new_z)
{
	set_rotation({new_x, new_y, new_z});
}

void Transform::set_position(vector3f new_pos)
{
	position = new_pos;

	recalculate_transformation_matrix();
	update_children();
}

void Transform::set_position(float new_x, float new_y, float new_z)
{
	set_position({new_x, new_y, new_z});
}

vector3f Transform::get_scale() const
{
	return scale;
}

vector3f Transform::get_rotation() const
{
	return rotation;
}

vector3f Transform::get_position() const
{
	return position;
}

mat4x4 Transform::get_transformation_matrix() const
{
	return transformation_matrix;
}

void Transform::recalculate_transformation_matrix()
{
	transformation_matrix = LMAT4X4_IDENTITY;

	transformation_matrix = transformation_matrix * mat4x4::scale(scale);

	transformation_matrix = transformation_matrix * mat4x4::euler_xyz(rotation.x, rotation.y, rotation.z);

	transformation_matrix = transformation_matrix * mat4x4::translation(position);
	
	if (parent != nullptr)
	{
		transformation_matrix = transformation_matrix * parent->transformation_matrix;
	}
}

void Transform::update_children()
{
	for (Transform* child : children)
	{
		child->recalculate_transformation_matrix();
		child->update_children();
	}
}

}
