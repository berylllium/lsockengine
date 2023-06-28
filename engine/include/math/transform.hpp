#pragma once

#include <vector>

#include "definitions.hpp"
#include "math/mat4x4.hpp"

namespace lise
{

class Transform
{
public:
	Transform();

	void set_parent(Transform* new_parent);
	
	void add_child(Transform* new_child);

	void set_scale(vector3f new_scale);
	void set_scale(float new_x, float new_y, float new_z);

	void set_rotation(vector3f new_rot);
	void set_rotation(float new_x, float new_y, float new_z);

	void set_position(vector3f new_pos);
	void set_position(float new_x, float new_y, float new_z);

	vector3f get_scale() const;
	vector3f get_rotation() const;
	vector3f get_position() const;

	mat4x4 get_transformation_matrix() const;

private:
	Transform* parent;
	std::vector<Transform*> children;

	vector3f scale;
	vector3f rotation;
	vector3f position;

	mat4x4 transformation_matrix;

	void recalculate_transformation_matrix();
	void update_children();
};

}
