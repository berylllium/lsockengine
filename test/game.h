#pragma once

#include <stdint.h>
#include <stdbool.h>

bool game_initialize();

bool game_update(float delta_time);

bool game_render(float delta_time);

void game_on_resize(uint32_t width, uint32_t height);
