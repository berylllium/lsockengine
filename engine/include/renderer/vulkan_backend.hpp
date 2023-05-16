#pragma once

#include <vulkan/vulkan.h>

#include "definitions.hpp"
#include "math/vector2.hpp"
#include "renderer/command_buffer.hpp"
#include "renderer/device.hpp"
#include "renderer/render_pass.hpp"
#include "renderer/swapchain.hpp"
#include "renderer/fence.hpp"
#include "renderer/vulkan_buffer.hpp"
#include "math/mat4x4.hpp"

namespace lise
{

bool vulkan_initialize(const char* application_name);

void vulkan_shutdown();

bool vulkan_begin_frame(float delta_time);

bool vulkan_end_frame(float delta_time);

vector2i vulkan_get_framebuffer_size();

// TODO: temp hack
LAPI void vulkan_set_view_matrix_temp(const mat4x4& view);

}
