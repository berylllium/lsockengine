#pragma once

#include "definitions.hpp"
#include "math/math.hpp"
#include "math/vector3.hpp"

namespace lise
{

struct mat4x4
{
	float data[16];

	mat4x4 operator * (const mat4x4& r) const;

	mat4x4 transposed() const;

	mat4x4 inversed() const;

	vector3f forward() const;
	vector3f backward() const;
	vector3f up() const;
	vector3f down() const;
	vector3f left() const;
	vector3f right() const;

	static mat4x4 orthographic(float left, float right, float top, float bottom, float near, float far);

	static mat4x4 perspective(float fov_rads, float aspect_ratio, float near, float far);

	static mat4x4 look_at(vector3f position, vector3f target, vector3f up);

	static mat4x4 translation(vector3f position);

	static mat4x4 scale(vector3f scale);

	static mat4x4 euler_x(float angle_radians);
	static mat4x4 euler_y(float angle_radians);
	static mat4x4 euler_z(float angle_radians);

	static mat4x4 euler_xyz(float x_radians, float y_radians, float z_radians);
};

#define LMAT4X4_ZERO ((mat4x4) 	{ 0, 0, 0, 0, \
								  0, 0, 0, 0, \
								  0, 0, 0, 0, \
								  0, 0, 0, 0 })

#define LMAT4X4_IDENTITY ((mat4x4) 	{ 1, 0, 0, 0, \
									  0, 1, 0, 0, \
									  0, 0, 1, 0, \
									  0, 0, 0, 1 })

}
