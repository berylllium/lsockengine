#pragma once

namespace lise
{

struct clock
{
	double start_time;

	clock();

	void reset();
	double get_elapsed_time();
};

}
