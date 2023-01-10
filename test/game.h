#pragma once

#include <stdint.h>

#include <game_type.h>

typedef struct game_state
{
	float delta_time;
} game_state;

int8_t game_initialize(game* game_instance);

int8_t game_update(game* game_instance, float delta_time);

int8_t game_render(game* game_instance, float delta_time);

void game_on_resize(game* game_instance, uint32_t width, uint32_t height);

