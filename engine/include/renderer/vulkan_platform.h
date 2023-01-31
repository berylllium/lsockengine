#pragma once

#include <vulkan/vulkan.h>

struct lise_platform_state;

bool lise_vulkan_platform_create_vulkan_surface(
    VkInstance instance,
    VkSurfaceKHR* out_surface);
