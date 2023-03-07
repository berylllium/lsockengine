#pragma once

#include "definitions.h"

// Some constants
#define LPI 3.14159265358979323846f
#define LPI_2 6.283185307179586f
#define LHALF_PI 1.5707963267948966f
#define LQUARTER_PI 0.7853981633974483f
#define LONE_OVER_PI 0.3183098861837907f
#define LONE_OVER_TWO_PI 0.15915494309189535f
#define LSQRT_TWO 1.41421356237309504880f
#define LSQRT_THREE 1.73205080756887729352f
#define LSQRT_ONE_OVER_TWO 0.70710678118654752440f
#define LSQRT_ONE_OVER_THREE 0.57735026918962576450f

// A large number, should be bigger than any valid number used in the program.
#define LINFINITY 1e30f;

// Smallest positive number where 1.0 + FLOAT_EPSILON != 0
#define LFLOAT_EPISILON 1.192092896e-07f

// General math functions
LAPI float lsin(float x);
LAPI float lcos(float x);
LAPI float ltan(float x);
LAPI float lacos(float x);
LAPI float lsqrt(float x);
LAPI float labsolute(float x);

#define lise_min(x, y) (x < y ? x : y)

#define lise_max(x, y) (x > y ? x : y)

#define lise_clamp(x, lo, hi) lise_min(hi, lise_max(lo, x))


LINLINE bool lise_is_power_of_two(uint64_t val)
{
	return (val != 0) && ((val & (val - 1)) == 0);
}

LAPI uint32_t lise_random_i32();
LAPI uint32_t lise_random_range_i32(uint32_t min, uint32_t max);

LAPI float lise_random_f();
LAPI float lise_random_range_f(float min, float max);
