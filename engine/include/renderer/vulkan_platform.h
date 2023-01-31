#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

struct lise_platform_state;

bool lise_vulkan_platform_create_vulkan_surface(struct lise_platform_state* plat_state,
                                                VkInstance instance,
                                                VkSurfaceKHR* out_surface);
