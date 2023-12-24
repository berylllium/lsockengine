#include "node/node3d.hpp"

namespace lise
{

void Node3D::set_scale(vector3f new_scale)
{
	_scale = new_scale;
	_set_transform_dirty();
}

void Node3D::set_scale(float new_x, float new_y, float new_z)
{
	set_scale({new_x, new_y, new_z});
}

void Node3D::set_rotation(vector3f new_rot)
{
	_rotation = new_rot;

	_set_transform_dirty();
}

void Node3D::set_rotation(float new_x, float new_y, float new_z)
{
	set_rotation({new_x, new_y, new_z});
}

void Node3D::set_position(vector3f new_pos)
{
	_position = new_pos;

	_set_transform_dirty();
}

void Node3D::set_position(float new_x, float new_y, float new_z)
{
	set_position({new_x, new_y, new_z});
}

const vector3f& Node3D::get_scale()
{
    if (_transform_dirty)
    {
        recalculate_transformation_matrix();
    }

	return _scale;
}

const vector3f& Node3D::get_rotation()
{
    if (_transform_dirty)
    {
        recalculate_transformation_matrix();
    }

	return _rotation;
}

const vector3f& Node3D::get_position()
{
    if (_transform_dirty)
    {
        recalculate_transformation_matrix();
    }

	return _position;
}

const mat4x4& Node3D::get_transformation_matrix()
{
    if (_transform_dirty)
    {
        recalculate_transformation_matrix();
    }

	return _transformation_matrix;
}

void Node3D::recalculate_transformation_matrix()
{
	_transformation_matrix = LMAT4X4_IDENTITY;

	_transformation_matrix = _transformation_matrix * mat4x4::scale(_scale);

	_transformation_matrix = _transformation_matrix * mat4x4::euler_xyz(_rotation.x, _rotation.y, _rotation.z);

	_transformation_matrix = _transformation_matrix * mat4x4::translation(_position);
    
    Node3D* parent = dynamic_cast<Node3D*>(_parent);

	if (parent)
	{
		_transformation_matrix = _transformation_matrix * parent->get_transformation_matrix();
	}

    _transform_dirty = false;
}

void Node3D::_set_transform_dirty()
{
    if (_transform_dirty)
        return;

    _transform_dirty = true;

    _first_child->propagate_notification_down(NOTIFICATION_TRANSFORM_DIRTIED);
}

void Node3D::_notification(int n)
{
    Node::_notification(n);

    switch (n)
    {
    case NOTIFICATION_TRANSFORM_DIRTIED:
    {
        _transform_dirty = true;
    } break;
    default: break;
    }
}

}
