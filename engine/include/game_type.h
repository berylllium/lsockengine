#pragma once

#include "core/application.h"

typedef struct game
{
	application_create_config app_create_config;

	int8_t (*initialize)(struct game* game_instance);
	int8_t (*update)(struct game* game_instance, float delta_time);
	int8_t (*render)(struct game* game_instance, float delta_time);
	void (*on_window_resize)(struct game* game_instance, uint32_t width, uint32_t height);

	void* custom_state;
} game;
