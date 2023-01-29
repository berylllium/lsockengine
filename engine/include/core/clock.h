#pragma once

typedef struct lise_clock
{
	double start_time;
} lise_clock;

void lise_clock_reset(lise_clock* clock);

double lise_clock_get_elapsed_time(lise_clock clock);
