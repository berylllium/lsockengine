#include "math/math.h"

#include <math.h>
#include <stdlib.h>

#include "definitions.h"
#include "platform/platform.h"

static bool is_rand_seeded = false;

float lsin(float x)
{
	return sinf(x);
}

float lcos(float x)
{
	return cosf(x);
}

float ltan(float x)
{
	return tanf(x);
}

float lacos(float x)
{
	return acosf(x);
}

float lsqrt(float x)
{
	return sqrtf(x);
}

float labsolute(float x)
{
	return fabs(x);
}

LAPI uint32_t lise_random_i32()
{
	if (!is_rand_seeded)
	{
		srand((uint32_t) lise_platform_get_absolute_time());
		is_rand_seeded = true;
	}

	return rand();
}

LAPI uint32_t lise_random_range_i32(uint32_t min, uint32_t max)
{
	if (!is_rand_seeded)
	{
		srand((uint32_t) lise_platform_get_absolute_time());
		is_rand_seeded = true;
	}

	return (rand() % (max - min + 1)) + min;
}

LAPI float lise_random_f()
{
	return (float) lise_random_i32() / (float) RAND_MAX;
}

LAPI float lise_random_range_f(float min, float max)
{
	return min + ((float) lise_random_i32() / ((float) RAND_MAX / (max - min)));
}
