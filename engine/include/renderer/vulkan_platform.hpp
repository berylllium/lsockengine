#pragma once

#include <vulkan/vulkan.h>

namespace lise
{

bool vulkan_platform_create_vulkan_surface(VkInstance instance, VkSurfaceKHR* out_surface);

}
