#pragma once

#include "definitions.hpp"

namespace lise
{

enum engine_event_codes
{
	ON_WINDOW_CLOSE,

	ON_MOUSE_BUTTON_DOWN,
	ON_MOUSE_BUTTON_UP,
	ON_MOUSE_MOVE,
	ON_MOUSE_WHEEL_MOVE,

	ON_KEY_DOWN,
	ON_KEY_UP,

	ON_WINDOW_RESIZE
};

struct event_context
{
	union
	{
		int64_t i64[2];
		uint64_t u64[2];
		double f64[2];

		int32_t i32[4];
		uint32_t u32[4];
		float f32[4];

		int16_t i16[8];
		uint16_t u16[8];

		int8_t i8[16];
		uint8_t u8[16];
		char c[16];
	} data;
};

typedef void (*on_event_cb)(uint16_t event_code, event_context ctx);

bool event_init();
void event_shutdown();

LAPI void event_register(uint16_t event_code);

LAPI void event_fire(uint16_t event_code, event_context ctx);

LAPI void event_add_listener(uint16_t event_code, on_event_cb listener);

}
