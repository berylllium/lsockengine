#include "game.hpp"

#include <core/logger.hpp>
#include <core/event.hpp>

bool game_initialize(lise::game* game_instance) 
{

	LFATAL("A test message: %f", 2.72f);
	LERROR("A test message: %f", 2.72f);
	LWARN("A test message: %f", 2.72f);
	LINFO("A test message: %f", 2.72f);
	LDEBUG("A test message: %f", 2.72f);
	LTRACE("A test message: %f", 2.72f);

	lise::event_add_listener(lise::engine_event_codes::ON_KEY_DOWN, [](uint16_t event_code, lise::event_context ctx) {
		LDEBUG("Key pressed: %d", ctx.data.u32[0]);
	});

	lise::event_add_listener(lise::engine_event_codes::ON_KEY_UP, [](uint16_t event_code, lise::event_context ctx) {
		LDEBUG("Key released: %d", ctx.data.u32[0]);
	});

	lise::event_add_listener(lise::engine_event_codes::ON_MOUSE_MOVE, [](uint16_t event_code, lise::event_context ctx) {
		LDEBUG("Mouse moved: %d, %d", ctx.data.u32[0], ctx.data.u32[1]);
	});

	lise::event_add_listener(lise::engine_event_codes::ON_MOUSE_WHEEL_MOVE, [](uint16_t event_code, lise::event_context ctx) {
		LDEBUG("Mouse wheel moved: %d", ctx.data.i8[0]);
	});
	
    return true;
}

bool game_update(lise::game* game_instance, float delta_time) 
{
	static float sum = 0;
	static int count = 0;

	sum += delta_time;
	count++;

	if (sum >= 1)
	{
		LDEBUG("Its been a second, %d game loops have passed. The average frametime was %f", count, sum / count);

		sum = 0;
		count = 0;
	}

    return true;
}

bool game_render(lise::game* game_instance, float delta_time) 
{
    return true;
}

void game_on_resize(lise::game* game_instance, uint32_t width, uint32_t height) 
{
}

