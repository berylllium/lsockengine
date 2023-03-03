#pragma once

#include "definitions.h"
#include "math/math.h"

// vec2i
typedef struct lise_vec2i
{
	uint32_t x, y;
} lise_vec2i;

#define lise_vec2_equals(l, r) ((l).x == (r).x && (l).y == (r).y)

// vec2
typedef struct lise_vec2
{
	union { float x, s, u; };
	union { float y, t, v; };
} lise_vec2;

#define LVEC2_ZERO ((lise_vec2) { 0, 0 })
#define LVEC2_ONE ((lise_vec2) { 1, 1 })

#define LVEC2_UP ((lise_vec2) { 0, 1 })
#define LVEC2_DOWN ((lise_vec2) { 0, -1 })
#define LVEC2_LEFT ((lise_vec2) { -1, 0 })
#define LVEC2_RIGHT ((lise_vec2) { 1, 0 })

#define lise_vec2_add(l, r) ((lise_vec2) { (l).x + (r).x, (l).y + (r).y })
#define lise_vec2_sub(l, r) ((lise_vec2) { (l).x - (r).x, (l).y - (r).y })
#define lise_vec2_mul(l, r) ((lise_vec2) { (l).x * (r).x, (l).y * (r).y })
#define lise_vec2_div(l, r) ((lise_vec2) { (l).x / (r).x, (l).y / (r).y })

#define lise_vec2_length_squared(v) ((v).x * (v).x + (v).y * (v).y)

#define lise_vec2_length(v) lsqrt(lise_vec2_length_squared(v))

LINLINE void lise_vec2_normalize(lise_vec2* v)
{
	float len = lise_vec2_length(*v);
	v->x /= len;
	v->y /= len;
}

LINLINE lise_vec2 lise_vec2_normalized(lise_vec2 v)
{
	lise_vec2_normalize(&v);

	return v;
}

LINLINE float lise_vec2_distance(lise_vec2 l, lise_vec2 r)
{
	lise_vec2 l_to_r = lise_vec2_sub(r, l);

	return lise_vec2_length(l_to_r);
}
