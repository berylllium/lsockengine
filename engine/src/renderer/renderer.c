#include "renderer/renderer.h"

#include "core/logger.h"
#include "renderer/vulkan_backend.h"

bool lise_renderer_initialize(lise_vector2i window_extent, const char* application_name)
{
    if (!lise_vulkan_initialize(window_extent, application_name))
    {
        LFATAL("Failed to initialize the vulkan backend.");
        return false;
    }

    return true;
}

void lise_renderer_shutdown()
{
    lise_vulkan_shutdown();
}
