#pragma once

#include "definitions.h"
#include "container/vector2.h"

bool lise_renderer_initialize(lise_vector2i window_extent, const char* application_name);

void lise_renderer_shutdown();

bool lise_renderer_draw_frame(float delta_time);
