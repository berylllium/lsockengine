#pragma once

struct clock
{
	double start_time;

	clock();

	double get_elapsed_time();
};