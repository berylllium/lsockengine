#pragma once

#include "definitions.h"

typedef struct lise_game_entry_points
{
	bool (*initialize)();
	bool (*update)(float delta_time);
	bool (*render)(float delta_time);
	void (*on_window_resize)(uint32_t width, uint32_t height);
} lise_game_entry_points;

typedef struct lise_application_create_info
{
	int16_t window_pos_x;
	int16_t window_pos_y;
	int16_t window_width;
	int16_t window_height;

	const char* window_name;
	
	lise_game_entry_points entry_points;
} lise_application_create_info;

LAPI bool lise_application_create(lise_application_create_info app_create_info);

LAPI bool lise_application_run();
