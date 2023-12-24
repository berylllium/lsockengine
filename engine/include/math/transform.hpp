#pragma once

#include <vector>

#include "definitions.hpp"
#include "math/mat4x4.hpp"

namespace lise
{

struct Transform
{
	void update(const Transform* parent);

	void set_scale(vector3f new_scale);
	void set_scale(float new_x, float new_y, float new_z);

	void set_rotation(vector3f new_rot);
	void set_rotation(float new_x, float new_y, float new_z);

	void set_position(vector3f new_pos);
	void set_position(float new_x, float new_y, float new_z);

	const vector3f& get_scale() const;
	const vector3f& get_rotation() const;
	const vector3f& get_position() const;

	const mat4x4& get_transformation_matrix() const;

	void recalculate_transformation_matrix(const Transform* parent);

	bool _dirty = true;

	vector3f _scale = LVEC3_ONE;
	vector3f _rotation = LVEC3_ZERO;
	vector3f _position = LVEC3_ZERO;

	mat4x4 _transformation_matrix;
};

}
