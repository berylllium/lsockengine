#pragma once

#include <vulkan/vulkan.h>

#include "definitions.h"
#include "container/vector2.h"
#include "renderer/device.h"
#include "renderer/swapchain.h"

typedef struct lise_vulkan_context
{
	VkInstance instance;

	VkSurfaceKHR surface;
	uint32_t framebuffer_width;
	uint32_t framebuffer_height;

	lise_device device;

	lise_swapchain swapchain;
} lise_vulkan_context;

bool lise_vulkan_initialize(lise_vector2i window_extent, const char* application_name);

void lise_vulkan_shutdown();
