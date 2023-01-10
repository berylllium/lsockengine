#include "game.h"

#include <core/logger.h>

int8_t game_initialize(game* game_instance) 
{

	LFATAL("A test message: %f", 3.14f);
	LERROR("A test message: %f", 3.14f);
	LWARN("A test message: %f", 3.14f);
	LINFO("A test message: %f", 3.14f);
	LDEBUG("A test message: %f", 3.14f);
	LTRACE("A test message: %f", 3.14f);

    return 1;
}

int8_t game_update(game* game_instance, float delta_time) 
{
    return 1;
}

int8_t game_render(game* game_instance, float delta_time) 
{
    return 1;
}

void game_on_resize(game* game_instance, uint32_t width, uint32_t height) 
{
}

