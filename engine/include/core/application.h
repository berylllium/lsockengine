#pragma once

#include "definitions.h"

struct game;

typedef struct application_create_config
{
	int16_t window_pos_x;
	int16_t window_pos_y;
	int16_t window_width;
	int16_t window_height;

	char* window_name;
} application_create_config;

LAPI int8_t application_create(struct game* game_instance);

LAPI int8_t application_run();
