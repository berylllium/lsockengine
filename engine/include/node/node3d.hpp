#pragma once

#include "math/mat4x4.hpp"
#include "node/node.hpp"

namespace lise
{

struct Node3D : public Node
{
    // Transform.
	void set_scale(vector3f new_scale);
	void set_scale(float new_x, float new_y, float new_z);

	void set_rotation(vector3f new_rot);
	void set_rotation(float new_x, float new_y, float new_z);

	void set_position(vector3f new_pos);
	void set_position(float new_x, float new_y, float new_z);

	const vector3f& get_scale();
	const vector3f& get_rotation();
	const vector3f& get_position();

	const mat4x4& get_transformation_matrix();

	void recalculate_transformation_matrix();

    void _set_transform_dirty();

    // Notification.

    virtual void _notification(int n) override;

    // Transform.
	bool _transform_dirty = true;

	vector3f _scale = LVEC3_ONE;
	vector3f _rotation = LVEC3_ZERO;
	vector3f _position = LVEC3_ZERO;

	mat4x4 _transformation_matrix;
};

}
