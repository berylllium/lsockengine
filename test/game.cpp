#include "game.hpp"

#include <core/logger.hpp>

bool game_initialize(game* game_instance) 
{

	LFATAL("A test message: %f", 3.14f);
	LERROR("A test message: %f", 3.14f);
	LWARN("A test message: %f", 3.14f);
	LINFO("A test message: %f", 3.14f);
	LDEBUG("A test message: %f", 3.14f);
	LTRACE("A test message: %f", 3.14f);
	
    return true;
}

bool game_update(game* game_instance, float delta_time) 
{
    return true;
}

bool game_render(game* game_instance, float delta_time) 
{
    return true;
}

void game_on_resize(game* game_instance, uint32_t width, uint32_t height) 
{
}

