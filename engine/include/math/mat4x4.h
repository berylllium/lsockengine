#pragma once

#include "definitions.h"
#include "math/math.h"
#include "math/vector3.h"

typedef struct lise_mat4x4
{
	float data[16];
} lise_mat4x4;

#define LMAT4X4_ZERO ((lise_mat4x4) { 0, 0, 0, 0, \
									  0, 0, 0, 0, \
									  0, 0, 0, 0, \
									  0, 0, 0, 0 })

#define LMAT4X4_IDENTITY ((lise_mat4x4) { 1, 0, 0, 0, \
										  0, 1, 0, 0, \
										  0, 0, 1, 0, \
										  0, 0, 0, 1 })

LINLINE lise_mat4x4 lise_mat4x4_mul(lise_mat4x4 l, lise_mat4x4 r)
{
	lise_mat4x4 out_matrix = LMAT4X4_IDENTITY;

	const float* l_ptr = l.data;
	const float* r_ptr = r.data;
	float* dst_ptr = out_matrix.data;

	for (int32_t i = 0; i < 4; ++i)
	{
		for (int32_t j = 0; j < 4; ++j) 
		{
			*dst_ptr =
				l_ptr[0] * r_ptr[0 + j] +
				l_ptr[1] * r_ptr[4 + j] +
				l_ptr[2] * r_ptr[8 + j] +
				l_ptr[3] * r_ptr[12 + j];
			dst_ptr++;
		}
		l_ptr += 4;
	}

	return out_matrix;
}

LINLINE lise_mat4x4 lise_mat4x4_orthographic(float left, float right, float top, float bottom, float near, float far)
{
	lise_mat4x4 out_matrix = LMAT4X4_IDENTITY;

	float lr = 1.0f / (left - right);
	float bt = 1.0f / (bottom - top);
	float nf = 1.0f / (near - far);

	out_matrix.data[0] = -2.0f * lr;
	out_matrix.data[5] = -2.0f * bt;
	out_matrix.data[10] = 2.0f * nf;

	out_matrix.data[12] = (left + right) * lr;
	out_matrix.data[13] = (top + bottom) * bt;
	out_matrix.data[14] = (far + near) * nf;

	return out_matrix;
}

LINLINE lise_mat4x4 lise_mat4x4_perspective(float fov_rads, float aspect_ratio, float near, float far) 
{
	float half_tan_fov = ltan(fov_rads * 0.5f);

	lise_mat4x4 out_matrix = LMAT4X4_ZERO;

	out_matrix.data[0] = 1.0f / (aspect_ratio * half_tan_fov);
	out_matrix.data[5] = 1.0f / half_tan_fov;
	out_matrix.data[10] = -((far + near) / (far - near));
	out_matrix.data[11] = -1.0f;
	out_matrix.data[14] = -((2.0f * far * near) / (far - near));

	return out_matrix;
}

LINLINE lise_mat4x4 lise_mat4x4_look_at(lise_vec3 position, lise_vec3 target, lise_vec3 up)
{
	lise_mat4x4 out_matrix;
	lise_vec3 z_axis;

	z_axis.x = target.x - position.x;
	z_axis.y = target.y - position.y;
	z_axis.z = target.z - position.z;

	z_axis = lise_vec3_normalized(z_axis);

	lise_vec3 x_axis = lise_vec3_normalized(lise_vec3_cross(z_axis, up));
	lise_vec3 y_axis = lise_vec3_cross(x_axis, z_axis);

	out_matrix.data[0] = x_axis.x;
	out_matrix.data[1] = y_axis.x;
	out_matrix.data[2] = -z_axis.x;
	out_matrix.data[3] = 0;
	out_matrix.data[4] = x_axis.y;
	out_matrix.data[5] = y_axis.y;
	out_matrix.data[6] = -z_axis.y;
	out_matrix.data[7] = 0;
	out_matrix.data[8] = x_axis.z;
	out_matrix.data[9] = y_axis.z;
	out_matrix.data[10] = -z_axis.z;
	out_matrix.data[11] = 0;
	out_matrix.data[12] = -lise_vec3_dot(x_axis, position);
	out_matrix.data[13] = -lise_vec3_dot(y_axis, position);
	out_matrix.data[14] = lise_vec3_dot(z_axis, position);
	out_matrix.data[15] = 1.0f;

	return out_matrix;
}

LINLINE lise_mat4x4 lise_mat4x4_transposed(lise_mat4x4 matrix)
{
	lise_mat4x4 out_matrix = LMAT4X4_IDENTITY;

	out_matrix.data[0] = matrix.data[0];
	out_matrix.data[1] = matrix.data[4];
	out_matrix.data[2] = matrix.data[8];
	out_matrix.data[3] = matrix.data[12];
	out_matrix.data[4] = matrix.data[1];
	out_matrix.data[5] = matrix.data[5];
	out_matrix.data[6] = matrix.data[9];
	out_matrix.data[7] = matrix.data[13];
	out_matrix.data[8] = matrix.data[2];
	out_matrix.data[9] = matrix.data[6];
	out_matrix.data[10] = matrix.data[10];
	out_matrix.data[11] = matrix.data[14];
	out_matrix.data[12] = matrix.data[3];
	out_matrix.data[13] = matrix.data[7];
	out_matrix.data[14] = matrix.data[11];
	out_matrix.data[15] = matrix.data[15];

	return out_matrix;
}

LINLINE lise_mat4x4 lise_mat4x4_inverse(lise_mat4x4 matrix) 
{
	const float* m = matrix.data;

	float t0 = m[10] * m[15];
	float t1 = m[14] * m[11];
	float t2 = m[6] * m[15];
	float t3 = m[14] * m[7];
	float t4 = m[6] * m[11];
	float t5 = m[10] * m[7];
	float t6 = m[2] * m[15];
	float t7 = m[14] * m[3];
	float t8 = m[2] * m[11];
	float t9 = m[10] * m[3];
	float t10 = m[2] * m[7];
	float t11 = m[6] * m[3];
	float t12 = m[8] * m[13];
	float t13 = m[12] * m[9];
	float t14 = m[4] * m[13];
	float t15 = m[12] * m[5];
	float t16 = m[4] * m[9];
	float t17 = m[8] * m[5];
	float t18 = m[0] * m[13];
	float t19 = m[12] * m[1];
	float t20 = m[0] * m[9];
	float t21 = m[8] * m[1];
	float t22 = m[0] * m[5];
	float t23 = m[4] * m[1];

	lise_mat4x4 out_matrix;
	float* o = out_matrix.data;

	o[0] = (t0 * m[5] + t3 * m[9] + t4 * m[13]) - (t1 * m[5] + t2 * m[9] + t5 * m[13]);
	o[1] = (t1 * m[1] + t6 * m[9] + t9 * m[13]) - (t0 * m[1] + t7 * m[9] + t8 * m[13]);
	o[2] = (t2 * m[1] + t7 * m[5] + t10 * m[13]) - (t3 * m[1] + t6 * m[5] + t11 * m[13]);
	o[3] = (t5 * m[1] + t8 * m[5] + t11 * m[9]) - (t4 * m[1] + t9 * m[5] + t10 * m[9]);

	float d = 1.0f / (m[0] * o[0] + m[4] * o[1] + m[8] * o[2] + m[12] * o[3]);

	o[0] = d * o[0];
	o[1] = d * o[1];
	o[2] = d * o[2];
	o[3] = d * o[3];
	o[4] = d * ((t1 * m[4] + t2 * m[8] + t5 * m[12]) - (t0 * m[4] + t3 * m[8] + t4 * m[12]));
	o[5] = d * ((t0 * m[0] + t7 * m[8] + t8 * m[12]) - (t1 * m[0] + t6 * m[8] + t9 * m[12]));
	o[6] = d * ((t3 * m[0] + t6 * m[4] + t11 * m[12]) - (t2 * m[0] + t7 * m[4] + t10 * m[12]));
	o[7] = d * ((t4 * m[0] + t9 * m[4] + t10 * m[8]) - (t5 * m[0] + t8 * m[4] + t11 * m[8]));
	o[8] = d * ((t12 * m[7] + t15 * m[11] + t16 * m[15]) - (t13 * m[7] + t14 * m[11] + t17 * m[15]));
	o[9] = d * ((t13 * m[3] + t18 * m[11] + t21 * m[15]) - (t12 * m[3] + t19 * m[11] + t20 * m[15]));
	o[10] = d * ((t14 * m[3] + t19 * m[7] + t22 * m[15]) - (t15 * m[3] + t18 * m[7] + t23 * m[15]));
	o[11] = d * ((t17 * m[3] + t20 * m[7] + t23 * m[11]) - (t16 * m[3] + t21 * m[7] + t22 * m[11]));
	o[12] = d * ((t14 * m[10] + t17 * m[14] + t13 * m[6]) - (t16 * m[14] + t12 * m[6] + t15 * m[10]));
	o[13] = d * ((t20 * m[14] + t12 * m[2] + t19 * m[10]) - (t18 * m[10] + t21 * m[14] + t13 * m[2]));
	o[14] = d * ((t18 * m[6] + t23 * m[14] + t15 * m[2]) - (t22 * m[14] + t14 * m[2] + t19 * m[6]));
	o[15] = d * ((t22 * m[10] + t16 * m[2] + t21 * m[6]) - (t20 * m[6] + t23 * m[10] + t17 * m[2]));

	return out_matrix;
}

LINLINE lise_mat4x4 lise_mat4x4_translation(lise_vec3 position)
{
	lise_mat4x4 out_matrix = LMAT4X4_IDENTITY;

	out_matrix.data[12] = position.x;
	out_matrix.data[13] = position.y;
	out_matrix.data[14] = position.z;

	return out_matrix;
}

LINLINE lise_mat4x4 lise_mat4x4_scale(lise_vec3 scale)
{
	lise_mat4x4 out_matrix = LMAT4X4_IDENTITY;

	out_matrix.data[0] = scale.x;
	out_matrix.data[5] = scale.y;
	out_matrix.data[10] = scale.z;

	return out_matrix;
}

LINLINE lise_mat4x4 lise_mat4x4_euler_x(float angle_radians)
{
	lise_mat4x4 out_matrix = LMAT4X4_IDENTITY;

	float c = lcos(angle_radians);
	float s = lsin(angle_radians);

	out_matrix.data[5] = c;
	out_matrix.data[6] = s;
	out_matrix.data[9] = -s;
	out_matrix.data[10] = c;

	return out_matrix;
}

LINLINE lise_mat4x4 lise_mat4x4_euler_y(float angle_radians)
{
	lise_mat4x4 out_matrix = LMAT4X4_IDENTITY;

	float c = lcos(angle_radians);
	float s = lsin(angle_radians);

	out_matrix.data[0] = c;
	out_matrix.data[2] = -s;
	out_matrix.data[8] = s;
	out_matrix.data[10] = c;

	return out_matrix;
}

LINLINE lise_mat4x4 lise_mat4x4_euler_z(float angle_radians)
{
	lise_mat4x4 out_matrix = LMAT4X4_IDENTITY;

	float c = lcos(angle_radians);
	float s = lsin(angle_radians);

	out_matrix.data[0] = c;
	out_matrix.data[1] = s;
	out_matrix.data[4] = -s;
	out_matrix.data[5] = c;

	return out_matrix;
}

LINLINE lise_mat4x4 lise_mat4x4_euler_xyz(float x_radians, float y_radians, float z_radians)
{
	lise_mat4x4 rx = lise_mat4x4_euler_x(x_radians);
	lise_mat4x4 ry = lise_mat4x4_euler_y(y_radians);
	lise_mat4x4 rz = lise_mat4x4_euler_z(z_radians);

	lise_mat4x4 out_matrix = lise_mat4x4_mul(rx, ry);
	out_matrix = lise_mat4x4_mul(out_matrix, rz);

	return out_matrix;
}

LINLINE lise_vec3 lise_mat4x4_forward(lise_mat4x4 matrix)
{
	lise_vec3 forward;

	forward.x = -matrix.data[2];
	forward.y = -matrix.data[6];
	forward.z = -matrix.data[10];

	lise_vec3_normalize(&forward);

	return forward;
}

LINLINE lise_vec3 lise_mat4x4_backward(lise_mat4x4 matrix)
{
	lise_vec3 backward;

	backward.x = matrix.data[2];
	backward.y = matrix.data[6];
	backward.z = matrix.data[10];

	lise_vec3_normalize(&backward);

	return backward;
}

LINLINE lise_vec3 lise_mat4x4_up(lise_mat4x4 matrix)
{
	lise_vec3 up;

	up.x = matrix.data[1];
	up.y = matrix.data[5];
	up.z = matrix.data[9];

	lise_vec3_normalize(&up);

	return up;
}

LINLINE lise_vec3 lise_mat4x4_down(lise_mat4x4 matrix)
{
	lise_vec3 down;

	down.x = -matrix.data[1];
	down.y = -matrix.data[5];
	down.z = -matrix.data[9];

	lise_vec3_normalize(&down);

	return down;
}

LINLINE lise_vec3 lise_mat4x4_left(lise_mat4x4 matrix)
{
	lise_vec3 right;

	right.x = -matrix.data[0];
	right.y = -matrix.data[4];
	right.z = -matrix.data[8];

	lise_vec3_normalize(&right);

	return right;
}

LINLINE lise_vec3 lise_mat4x4_right(lise_mat4x4 matrix)
{
	lise_vec3 left;

	left.x = matrix.data[0];
	left.y = matrix.data[4];
	left.z = matrix.data[8];

	lise_vec3_normalize(&left);

	return left;
}
