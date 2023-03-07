#pragma once

#include <vulkan/vulkan.h>

#include "definitions.h"
#include "math/vector2.h"
#include "renderer/command_buffer.h"
#include "renderer/device.h"
#include "renderer/render_pass.h"
#include "renderer/swapchain.h"
#include "renderer/fence.h"
#include "renderer/shader/object_shader.h"
#include "renderer/vulkan_buffer.h"

typedef struct lise_vulkan_context
{
	VkInstance instance;

	VkSurfaceKHR surface;

	lise_device device;

	lise_swapchain swapchain;
	lise_render_pass render_pass;

	lise_command_buffer* graphics_command_buffers;

	VkSemaphore* image_available_semaphores;

	VkSemaphore* queue_complete_semaphores;

	uint32_t in_flight_fence_count;
	lise_fence* in_flight_fences;

	// Fences in this array are not owned by the array
	lise_fence** images_in_flight;

	uint32_t current_image_index;

	lise_object_shader object_shader;

	lise_vulkan_buffer object_vertex_buffer;
	lise_vulkan_buffer object_index_buffer;
} lise_vulkan_context;

bool lise_vulkan_initialize(const char* application_name);

void lise_vulkan_shutdown();

bool lise_vulkan_begin_frame(float delta_time);

bool lise_vulkan_end_frame(float delta_time);

lise_vec2i lise_vulkan_get_framebuffer_size();

// TODO: temp hack
LAPI void lise_vulkan_set_view_matrix_temp(lise_mat4x4 view);
