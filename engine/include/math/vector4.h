#pragma once

#include "definitions.h"
#include "math/math.h"

typedef struct lise_vec4
{
	union { float x, r; };
	union { float y, g; };
	union { float z, b; };
	union { float w, a; };
} lise_vec4;

#define LVEC4_ZERO ((lise_vec4) { 0, 0, 0, 0 })
#define LVEC4_ONE ((lise_vec4) { 1, 1, 1, 1 })

#define LVEC4_UP ((lise_vec4) { 0, 1, 0, 0 })
#define LVEC4_DOWN ((lise_vec4) { 0, -1, 0, 0 })
#define LVEC4_LEFT ((lise_vec4) { -1, 0, 0, 0 })
#define LVEC4_RIGHT ((lise_vec4) { 1, 0, 0, 0 })

#define lise_vec4_add(l, r) ((lise_vec4) { (l).x + (r).x, (l).y + (r).y, (l).z + (r).z, (l).w + (r).w })
#define lise_vec4_sub(l, r) ((lise_vec4) { (l).x - (r).x, (l).y - (r).y, (l).z - (r).z, (l).w - (r).w })
#define lise_vec4_mul(l, r) ((lise_vec4) { (l).x * (r).x, (l).y * (r).y, (l).z * (r).z, (l).w * (r).w })
#define lise_vec4_div(l, r) ((lise_vec4) { (l).x / (r).x, (l).y / (r).y, (l).z / (r).z, (l).w / (r).w })

#define lise_vec4_length_squared(v) ((v).x * (v).x + (v).y * (v).y + (v).z * (v).z)

#define lise_vec4_length(v) lsqrt(lise_vec4_length_squared(v))

LINLINE void lise_vec4_normalize(lise_vec4* v)
{
	float len = lise_vec4_length(*v);
	v->x /= len;
	v->y /= len;
	v->z /= len;
}

LINLINE lise_vec4 lise_vec4_normalized(lise_vec4 v)
{
	lise_vec4_normalize(&v);

	return v;
}

LINLINE float lise_vec3_distance(lise_vec4 l, lise_vec4 r)
{
	lise_vec4 l_to_r = lise_vec4_sub(r, l);

	return lise_vec4_length(l_to_r);
}
