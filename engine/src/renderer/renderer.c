#include "renderer/renderer.h"

#include "core/logger.h"
#include "renderer/vulkan_backend.h"

bool lise_renderer_initialize(const char* application_name)
{
	if (!lise_vulkan_initialize(application_name))
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

bool lise_renderer_draw_frame(float delta_time)
{
	// Begin the frame
	if (!lise_vulkan_begin_frame(delta_time))
	{
		LFATAL("Failed begin the frame drawing process.");
		return false;
	}

	// Do mid-frame stuff

	// End the frame
	if (!lise_vulkan_end_frame(delta_time))
	{
		LFATAL("Failed to end the frame drawing process.");
		return false;
	}

	return true;
}
