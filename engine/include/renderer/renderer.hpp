#pragma once

#include "definitions.hpp"

namespace lise
{

bool renderer_initialize(const char* consumer_name);

void renderer_shutdown();

bool renderer_draw_frame(float delta_time);

}
