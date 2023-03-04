#pragma once

#include "math/vector4.h"
#include "math/mat4x4.h"
#include "math/math.h"

typedef lise_vec4 lise_quat;

#define LQUAT_IDENTITY ((lise_quat) { 0.0f, 0.0f, 0.0f, 1.0f })

LINLINE float lise_quat_normal(lise_quat q)
{
	return lsqrt(
		q.x * q.x +
		q.y * q.y +
		q.z * q.z +
		q.w * q.w
	);
}

LINLINE lise_quat lise_quat_normalize(lise_quat q)
{
	float normal = lise_quat_normal(q);

	return (lise_quat) {
		q.x / normal,
		q.y / normal,
		q.z / normal,
		q.w / normal
	};
}

LINLINE lise_quat lise_quat_conjugate(lise_quat q)
{
	return (lise_quat) {
		-q.x,
		-q.y,
		-q.z,
		q.w
	};
}

LINLINE lise_quat lise_quat_inverse(lise_quat q)
{
	return lise_quat_normalize(lise_quat_conjugate(q));
}

LINLINE lise_quat lise_quat_mul(lise_quat l, lise_quat r)
{
	lise_quat out_quaternion;

	out_quaternion.x = l.x * r.w +
					   l.y * r.z -
					   l.z * r.y +
					   l.w * r.x;

	out_quaternion.y = -l.x * r.z +
					   l.y * r.w +
					   l.z * r.x +
					   l.w * r.y;

	out_quaternion.z = l.x * r.y -
					   l.y * r.x +
					   l.z * r.w +
					   l.w * r.z;

	out_quaternion.w = -l.x * r.x -
					   l.y * r.y -
					   l.z * r.z +
					   l.w * r.w;

	return out_quaternion;
}
