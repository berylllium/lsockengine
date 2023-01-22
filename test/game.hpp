#pragma once

#include <cstdint>

#include <game_type.hpp>

struct game_state
{
	float delta_time;
};

bool game_initialize(lise::game* game_instance);

bool game_update(lise::game* game_instance, float delta_time);

bool game_render(lise::game* game_instance, float delta_time);

void game_on_resize(lise::game* game_instance, uint32_t width, uint32_t height);

