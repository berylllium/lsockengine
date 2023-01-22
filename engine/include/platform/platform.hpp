#pragma once

#include "definitions.hpp"

namespace lise
{

struct platform_state
{
	void* internal_state;
};

LAPI bool platform_init(
	platform_state* plat_state,
	const char* application_name,
	int32_t x, int32_t y,
	int32_t width, int32_t height);

LAPI void platform_shutdown(platform_state* plat_state);

LAPI bool platform_poll_messages(platform_state* plat_state);

void platform_console_write(const char* message, uint8_t color);
void platform_console_write_error(const char* message, uint8_t color);

double platform_get_absolute_time();

void platform_sleep(uint64_t ms);

}
