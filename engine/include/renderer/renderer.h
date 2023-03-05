#pragma once

#include "definitions.h"

bool lise_renderer_initialize(const char* application_name);

void lise_renderer_shutdown();

bool lise_renderer_draw_frame(float delta_time);
