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

LINLINE float lise_quat_dot(lise_quat r, lise_quat l)
{
    return r.x * l.x +
           r.y * l.y +
           r.z * l.z +
           r.w * l.w;
}

LINLINE lise_mat4x4 lise_quat_to_mat4x4(lise_quat q)
{
    lise_mat4x4 out_matrix = LMAT4X4_IDENTITY;

    // https://stackoverflow.com/questions/1556260/convert-quaternion-rotation-to-rotation-matrix

    lise_quat n = lise_quat_normalize(q);

    out_matrix.data[0] = 1.0f - 2.0f * n.y * n.y - 2.0f * n.z * n.z;
    out_matrix.data[1] = 2.0f * n.x * n.y - 2.0f * n.z * n.w;
    out_matrix.data[2] = 2.0f * n.x * n.z + 2.0f * n.y * n.w;

    out_matrix.data[4] = 2.0f * n.x * n.y + 2.0f * n.z * n.w;
    out_matrix.data[5] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.z * n.z;
    out_matrix.data[6] = 2.0f * n.y * n.z - 2.0f * n.x * n.w;

    out_matrix.data[8] = 2.0f * n.x * n.z - 2.0f * n.y * n.w;
    out_matrix.data[9] = 2.0f * n.y * n.z + 2.0f * n.x * n.w;
    out_matrix.data[10] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.y * n.y;

    return out_matrix;
}

// Calculates a rotation matrix based on the lise_quaternion and the passed in center point.
LINLINE lise_mat4x4 lise_quat_to_rotation_matrix(lise_quat q, lise_vec3 center)
{
    lise_mat4x4 out_matrix;

    float* o = out_matrix.data;
    o[0] = (q.x * q.x) - (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
    o[1] = 2.0f * ((q.x * q.y) + (q.z * q.w));
    o[2] = 2.0f * ((q.x * q.z) - (q.y * q.w));
    o[3] = center.x - center.x * o[0] - center.y * o[1] - center.z * o[2];

    o[4] = 2.0f * ((q.x * q.y) - (q.z * q.w));
    o[5] = -(q.x * q.x) + (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
    o[6] = 2.0f * ((q.y * q.z) + (q.x * q.w));
    o[7] = center.y - center.x * o[4] - center.y * o[5] - center.z * o[6];

    o[8] = 2.0f * ((q.x * q.z) + (q.y * q.w));
    o[9] = 2.0f * ((q.y * q.z) - (q.x * q.w));
    o[10] = -(q.x * q.x) - (q.y * q.y) + (q.z * q.z) + (q.w * q.w);
    o[11] = center.z - center.x * o[8] - center.y * o[9] - center.z * o[10];

    o[12] = 0.0f;
    o[13] = 0.0f;
    o[14] = 0.0f;
    o[15] = 1.0f;

    return out_matrix;
}

LINLINE lise_quat lise_quat_from_axis_angle(lise_vec3 axis, float angle, bool normalize)
{
    const float half_angle = 0.5f * angle;

    float s = lsin(half_angle);
    float c = lcos(half_angle);

    lise_quat q = (lise_quat){s * axis.x, s * axis.y, s * axis.z, c};

    if (normalize)
	{
        return lise_quat_normalize(q);
    }

    return q;
}

LINLINE lise_quat lise_quat_slerp(lise_quat q_0, lise_quat q_1, float percentage)
{
    lise_quat out_lise_quaternion;

    // Source: https://en.wikipedia.org/wiki/Slerp
    // Only unit lise_quaternions are valid rotations.
    // Normalize to avoid undefined behavior.
    lise_quat v0 = lise_quat_normalize(q_0);
    lise_quat v1 = lise_quat_normalize(q_1);

    // Compute the cosine of the angle between the two vectors.
    float dot = lise_quat_dot(v0, v1);

    // If the dot product is negative, slerp won't take
    // the shorter path. Note that v1 and -v1 are equivalent when
    // the negation is applied to all four components. Fix by
    // reversing one lise_quaternion.
    if (dot < 0.0f) {
        v1.x = -v1.x;
        v1.y = -v1.y;
        v1.z = -v1.z;
        v1.w = -v1.w;
        dot = -dot;
    }

    const float DOT_THRESHOLD = 0.9995f;
    if (dot > DOT_THRESHOLD) {
        // If the inputs are too close for comfort, linearly interpolate
        // and normalize the result.
        out_lise_quaternion = (lise_quat){
            v0.x + ((v1.x - v0.x) * percentage),
            v0.y + ((v1.y - v0.y) * percentage),
            v0.z + ((v1.z - v0.z) * percentage),
            v0.w + ((v1.w - v0.w) * percentage)};

        return lise_quat_normalize(out_lise_quaternion);
    }

    // Since dot is in range [0, DOT_THRESHOLD], acos is safe
    float theta_0 = lacos(dot);          // theta_0 = angle between input vectors
    float theta = theta_0 * percentage;  // theta = angle between v0 and result
    float sin_theta = lsin(theta);       // compute this value only once
    float sin_theta_0 = lsin(theta_0);   // compute this value only once

    float s0 = lcos(theta) - dot * sin_theta / sin_theta_0;  // == sin(theta_0 - theta) / sin(theta_0)
    float s1 = sin_theta / sin_theta_0;

    return (lise_quat)
	{
        (v0.x * s0) + (v1.x * s1),
        (v0.y * s0) + (v1.y * s1),
        (v0.z * s0) + (v1.z * s1),
        (v0.w * s0) + (v1.w * s1)
	};
}
