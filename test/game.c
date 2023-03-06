#include "game.h"

#include <core/logger.h>
#include <core/event.h>

void on_event(uint16_t event_code, lise_event_context ctx)
{
	switch (event_code)
	{
	case LISE_EVENT_ON_KEY_DOWN:
		LDEBUG("Key pressed: %d", ctx.data.u32[0]);
	break;
	case LISE_EVENT_ON_KEY_UP:
		LDEBUG("Key released: %d", ctx.data.u32[0]);
	break;
	case LISE_EVENT_ON_MOUSE_MOVE:
		LDEBUG("Mouse moved: %d, %d", ctx.data.u32[0], ctx.data.u32[1]);
	break;
	case LISE_EVENT_ON_MOUSE_WHEEL_MOVE:
		LDEBUG("Mouse wheel moved: %d", ctx.data.i8[0]);
	break;
	}
}

void on_window_resize(uint16_t event_code, lise_event_context ctx)
{
	LDEBUG("New window size: %d, %d", ctx.data.u32[0], ctx.data.u32[1]);
}

bool game_initialize() 
{

	LFATAL("A test message: %f", 2.72f);
	LERROR("A test message: %f", 2.72f);
	LWARN("A test message: %f", 2.72f);
	LINFO("A test message: %f", 2.72f);
	LDEBUG("A test message: %f", 2.72f);
	LTRACE("A test message: %f", 2.72f);

	lise_event_add_listener(LISE_EVENT_ON_KEY_DOWN, on_event);

	lise_event_add_listener(LISE_EVENT_ON_KEY_UP, on_event);

	//lise_event_add_listener(LISE_EVENT_ON_MOUSE_MOVE, on_event);

	lise_event_add_listener(LISE_EVENT_ON_MOUSE_WHEEL_MOVE, on_event);

	lise_event_add_listener(LISE_EVENT_ON_WINDOW_RESIZE, on_window_resize);
	
    return true;
}

bool game_update(float delta_time) 
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

bool game_render(float delta_time) 
{
    return true;
}

void game_on_resize(uint32_t width, uint32_t height) 
{
}
