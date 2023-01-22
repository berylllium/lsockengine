#pragma once

#include "core/application.hpp"

namespace lise
{

struct game
{
	application_create_config app_create_config;

	bool (*initialize)(struct game* game_instance);
	bool (*update)(struct game* game_instance, float delta_time);
	bool (*render)(struct game* game_instance, float delta_time);
	void (*on_window_resize)(struct game* game_instance, uint32_t width, uint32_t height);

	void* custom_state;
};

}
