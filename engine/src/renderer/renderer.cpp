#include "renderer/renderer.hpp"

#include "core/logger.hpp"
#include "renderer/vulkan_backend.hpp"

namespace lise
{

bool renderer_initialize(const char* consumer_name)
{
	if (!vulkan_initialize(consumer_name))
	{
		LFATAL("Failed to initialize the vulkan backend.");
		return false;
	}

	return true;
}

void renderer_shutdown()
{
	vulkan_shutdown();
}

bool renderer_draw_frame(float delta_time)
{
	// Begin the frame
	if (!vulkan_begin_frame(delta_time))
	{
		LFATAL("Failed begin the frame drawing process.");
		return false;
	}

	// Do mid-frame stuff

	// End the frame
	if (!vulkan_end_frame(delta_time))
	{
		LFATAL("Failed to end the frame drawing process.");
		return false;
	}

	return true;
}

}
