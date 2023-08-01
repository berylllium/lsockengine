#include "math/mat4x4.hpp"

namespace lise
{

mat4x4 mat4x4::operator * (const mat4x4& r) const
{
	auto out_matrix = LMAT4X4_IDENTITY;

	auto l_ptr = data;
	auto r_ptr = r.data;
	auto dst_ptr = out_matrix.data;

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

mat4x4 mat4x4::transposed() const
{
	auto out_matrix = LMAT4X4_IDENTITY;

	out_matrix.data[0] = data[0];
	out_matrix.data[1] = data[4];
	out_matrix.data[2] = data[8];
	out_matrix.data[3] = data[12];
	out_matrix.data[4] = data[1];
	out_matrix.data[5] = data[5];
	out_matrix.data[6] = data[9];
	out_matrix.data[7] = data[13];
	out_matrix.data[8] = data[2];
	out_matrix.data[9] = data[6];
	out_matrix.data[10] = data[10];
	out_matrix.data[11] = data[14];
	out_matrix.data[12] = data[3];
	out_matrix.data[13] = data[7];
	out_matrix.data[14] = data[11];
	out_matrix.data[15] = data[15];

	return out_matrix;
}

mat4x4 mat4x4::inversed() const
{
	auto m = data;

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

	mat4x4 out_matrix;
	auto o = out_matrix.data;

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

vector3f mat4x4::forward() const
{
	vector3f forward;

	forward.x = -data[2];
	forward.y = -data[6];
	forward.z = -data[10];

	forward.normalize();

	return forward;
}

vector3f mat4x4::backward() const
{
	vector3f backward;

	backward.x = data[2];
	backward.y = data[6];
	backward.z = data[10];

	backward.normalize();

	return backward;
}

vector3f mat4x4::up() const
{
	vector3f up;

	up.x = data[1];
	up.y = data[5];
	up.z = data[9];

	up.normalize();

	return up;
}

vector3f mat4x4::down() const
{
	vector3f down;

	down.x = -data[1];
	down.y = -data[5];
	down.z = -data[9];

	down.normalize();

	return down;
}

vector3f mat4x4::left() const
{
	vector3f right;

	right.x = -data[0];
	right.y = -data[4];
	right.z = -data[8];

	right.normalize();

	return right;
}

vector3f mat4x4::right() const
{
	vector3f left;

	left.x = data[0];
	left.y = data[4];
	left.z = data[8];

	left.normalize();

	return left;
}

mat4x4 mat4x4::orthographic(float left, float right, float top, float bottom, float near, float far)
{
	mat4x4 out_matrix = LMAT4X4_IDENTITY;

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

mat4x4 mat4x4::perspective(float fov_rads, float aspect_ratio, float near, float far)
{
	float half_tan_fov = tan(fov_rads * 0.5f);

	mat4x4 out_matrix = LMAT4X4_ZERO;

	out_matrix.data[0] = 1.0f / (aspect_ratio * half_tan_fov);
	out_matrix.data[5] = 1.0f / half_tan_fov;
	out_matrix.data[10] = -((far + near) / (far - near));
	out_matrix.data[11] = -1.0f;
	out_matrix.data[14] = -((2.0f * far * near) / (far - near));

	return out_matrix;
}

mat4x4 mat4x4::look_at(vector3f position, vector3f target, vector3f up)
{
	mat4x4 out_matrix;
	vector3f z_axis;

	z_axis.x = target.x - position.x;
	z_axis.y = target.y - position.y;
	z_axis.z = target.z - position.z;

	z_axis.normalize();

	vector3f x_axis = x_axis.cross(up).normalized();
	vector3f y_axis = x_axis.cross(z_axis);

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
	out_matrix.data[12] = -x_axis.dot(position);
	out_matrix.data[13] = -y_axis.dot(position);
	out_matrix.data[14] = z_axis.dot(position);
	out_matrix.data[15] = 1.0f;

	return out_matrix;
}

mat4x4 mat4x4::translation(vector3f position)
{
	mat4x4 out_matrix = LMAT4X4_IDENTITY;

	out_matrix.data[12] = position.x;
	out_matrix.data[13] = position.y;
	out_matrix.data[14] = position.z;

	return out_matrix;
}

mat4x4 mat4x4::scale(vector3f scale)
{
	mat4x4 out_matrix = LMAT4X4_IDENTITY;

	out_matrix.data[0] = scale.x;
	out_matrix.data[5] = scale.y;
	out_matrix.data[10] = scale.z;

	return out_matrix;
}

mat4x4 mat4x4::euler_x(float angle_radians)
{
	mat4x4 out_matrix = LMAT4X4_IDENTITY;

	float c = cos(angle_radians);
	float s = sin(angle_radians);

	out_matrix.data[5] = c;
	out_matrix.data[6] = s;
	out_matrix.data[9] = -s;
	out_matrix.data[10] = c;

	return out_matrix;
}

mat4x4 mat4x4::euler_y(float angle_radians)
{
	mat4x4 out_matrix = LMAT4X4_IDENTITY;

	float c = cos(angle_radians);
	float s = sin(angle_radians);

	out_matrix.data[0] = c;
	out_matrix.data[2] = -s;
	out_matrix.data[8] = s;
	out_matrix.data[10] = c;

	return out_matrix;
}

mat4x4 mat4x4::euler_z(float angle_radians)
{
	mat4x4 out_matrix = LMAT4X4_IDENTITY;

	float c = cos(angle_radians);
	float s = sin(angle_radians);

	out_matrix.data[0] = c;
	out_matrix.data[1] = s;
	out_matrix.data[4] = -s;
	out_matrix.data[5] = c;

	return out_matrix;
}

mat4x4 mat4x4::euler_xyz(float x_radians, float y_radians, float z_radians)
{
	mat4x4 rx = euler_x(x_radians);
	mat4x4 ry = euler_y(y_radians);
	mat4x4 rz = euler_z(z_radians);

	return rx * ry * rz;
}

}
