#pragma once

#include <optional>

#include <vulkan/vulkan.hpp>

namespace lise
{

std::optional<vk::SurfaceKHR> vulkan_platform_create_vulkan_surface(vk::Instance instance);

}
