#pragma once

#include <vulkan/vulkan.h>

#include "definitions.h"

typedef struct lise_vulkan_context
{
	VkInstance instance;
} lise_vulkan_context;

bool lise_vulkan_initialize(const char* application_name);

void lise_vulkan_shutdown();
