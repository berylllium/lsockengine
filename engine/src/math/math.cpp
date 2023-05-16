#include "math/math.hpp"

#include <cmath>
#include <cstdlib>

#include "platform/platform.hpp"
#include "definitions.hpp"

static bool is_rand_seeded = false;

namespace lise
{

float sin(float x)
{
	return sinf(x);
}

float cos(float x)
{
	return cosf(x);
}

float tan(float x)
{
	return tanf(x);
}

float acos(float x)
{
	return acosf(x);
}

float sqrt(float x)
{
	return sqrtf(x);
}

float absolute(float x)
{
	return fabs(x);
}

LAPI uint32_t random_i32()
{
	if (!is_rand_seeded)
	{
		srand((uint32_t) platform_get_absolute_time());
		is_rand_seeded = true;
	}

	return rand();
}

LAPI uint32_t random_range_i32(uint32_t min, uint32_t max)
{
	if (!is_rand_seeded)
	{
		srand((uint32_t) platform_get_absolute_time());
		is_rand_seeded = true;
	}

	return (rand() % (max - min + 1)) + min;
}

LAPI float random_f()
{
	return (float) random_i32() / (float) RAND_MAX;
}

LAPI float random_range_f(float min, float max)
{
	return min + ((float) random_i32() / ((float) RAND_MAX / (max - min)));
}

}
