#include "math/transform.hpp"

namespace lise
{

void Transform::set_scale(vector3f new_scale)
{
	_scale = new_scale;
	_dirty = true;
}

void Transform::set_scale(float new_x, float new_y, float new_z)
{
	set_scale({new_x, new_y, new_z});
}

void Transform::set_rotation(vector3f new_rot)
{
	_rotation = new_rot;

	_dirty = true;
}

void Transform::set_rotation(float new_x, float new_y, float new_z)
{
	set_rotation({new_x, new_y, new_z});
}

void Transform::set_position(vector3f new_pos)
{
	_position = new_pos;

	_dirty = true;
}

void Transform::set_position(float new_x, float new_y, float new_z)
{
	set_position({new_x, new_y, new_z});
}

const vector3f& Transform::get_scale() const
{
	return _scale;
}

const vector3f& Transform::get_rotation() const
{
	return _rotation;
}

const vector3f& Transform::get_position() const
{
	return _position;
}

const mat4x4& Transform::get_transformation_matrix() const
{
	return _transformation_matrix;
}

void Transform::recalculate_transformation_matrix(const Transform* parent)
{
	_transformation_matrix = LMAT4X4_IDENTITY;

	_transformation_matrix = _transformation_matrix * mat4x4::scale(_scale);

	_transformation_matrix = _transformation_matrix * mat4x4::euler_xyz(_rotation.x, _rotation.y, _rotation.z);

	_transformation_matrix = _transformation_matrix * mat4x4::translation(_position);
	
	if (parent != nullptr)
	{
		_transformation_matrix = _transformation_matrix * parent->_transformation_matrix;
	}
}

}
