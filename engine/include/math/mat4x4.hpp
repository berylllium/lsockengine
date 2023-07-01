#pragma once

#include "definitions.hpp"
#include "math/math.hpp"
#include "math/vector3.hpp"

namespace lise
{

struct mat4x4
{
	float data[16];

	LAPI mat4x4 operator * (const mat4x4& r) const;

	LAPI mat4x4 transposed() const;

	LAPI mat4x4 inversed() const;

	LAPI vector3f forward() const;
	LAPI vector3f backward() const;
	LAPI vector3f up() const;
	LAPI vector3f down() const;
	LAPI vector3f left() const;
	LAPI vector3f right() const;

	LAPI static mat4x4 orthographic(float left, float right, float top, float bottom, float near, float far);

	LAPI static mat4x4 perspective(float fov_rads, float aspect_ratio, float near, float far);

	LAPI static mat4x4 look_at(vector3f position, vector3f target, vector3f up);

	LAPI static mat4x4 translation(vector3f position);

	LAPI static mat4x4 scale(vector3f scale);

	LAPI static mat4x4 euler_x(float angle_radians);
	LAPI static mat4x4 euler_y(float angle_radians);
	LAPI static mat4x4 euler_z(float angle_radians);

	LAPI static mat4x4 euler_xyz(float x_radians, float y_radians, float z_radians);
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
