#include "core/clock.h"

#include "platform/platform.h"

void lise_clock_reset(lise_clock* clock)
{
	clock->start_time = lise_platform_get_absolute_time();
}

double lise_clock_get_elapsed_time(lise_clock clock)
{
	return lise_platform_get_absolute_time() - clock.start_time;
}
