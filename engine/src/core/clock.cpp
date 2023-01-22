#include "core/clock.hpp"

#include "platform/platform.hpp"

clock::clock()
{
	start_time = platform_get_absolute_time();
}

double clock::get_elapsed_time()
{
	return platform_get_absolute_time() - start_time;
}
