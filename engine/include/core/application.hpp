#pragma once

#include "definitions.hpp"

struct game;

struct application_create_config
{
	int16_t window_pos_x;
	int16_t window_pos_y;
	int16_t window_width;
	int16_t window_height;

	const char* window_name;
};

LAPI bool application_create(game* game_instance);

LAPI bool application_run();
