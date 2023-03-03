#pragma once

#include "definitions.h"
#include "math/math.h"

typedef struct lise_vec3
{
	union { float x, r, u; };
	union { float y, g, v; };
	union { float z, b, w; };
} lise_vec3;

#define LVEC3_ZERO ((lise_vec3) { 0, 0, 0 })
#define LVEC3_ONE ((lise_vec3) { 1, 1, 1 })

#define LVEC3_UP ((lise_vec3) { 0, 1, 0 })
#define LVEC3_DOWN ((lise_vec3) { 0, -1, 0 })
#define LVEC3_LEFT ((lise_vec3) { -1, 0, 0 })
#define LVEC3_RIGHT ((lise_vec3) { 1, 0, 0 })

#define lise_vec3_add(l, r) ((lise_vec3) { (l).x + (r).x, (l).y + (r).y, (l).z + (r).z})
#define lise_vec3_sub(l, r) ((lise_vec3) { (l).x - (r).x, (l).y - (r).y, (l).z - (r).z})
#define lise_vec3_mul(l, r) ((lise_vec3) { (l).x * (r).x, (l).y * (r).y, (l).z * (r).z})
#define lise_vec3_div(l, r) ((lise_vec3) { (l).x / (r).x, (l).y / (r).y, (l).z / (r).z})

#define lise_vec3_length_squared(v) ((v).x * (v).x + (v).y * (v).y + (v).z * (v).z)

#define lise_vec3_length(v) lsqrt(lise_vec3_length_squared(v))

LINLINE void lise_vec3_normalize(lise_vec3* v)
{
	float len = lise_vec3_length(*v);
	v->x /= len;
	v->y /= len;
	v->z /= len;
}

LINLINE lise_vec3 lise_vec3_normalized(lise_vec3 v)
{
	lise_vec3_normalize(&v);

	return v;
}

LINLINE float lise_vec3_distance(lise_vec3 l, lise_vec3 r)
{
	lise_vec3 l_to_r = lise_vec3_sub(r, l);

	return lise_vec3_length(l_to_r);
}

LINLINE float lise_vec3_dot(lise_vec3 l, lise_vec3 r) {
	float p = 0;

	p += l.x * r.x;
	p += l.y * r.y;
	p += l.z * r.z;

	return p;
}

LINLINE lise_vec3 lise_vec3_cross(lise_vec3 l, lise_vec3 r)
{
	return (lise_vec3)
	{
		l.y * r.z - l.z * r.y,
		l.z * r.x - l.x * r.z,
		l.x * r.y - l.y * r.x
	};
}
