#include "renderer/vulkan_backend.h"

#include <string.h>
#include <stdlib.h>

#include "core/logger.h"
#include "platform/platform.h"
#include "renderer/vulkan_platform.h"

#ifdef NDEBUG
	static const bool enable_validation_layers = false;
#else
	static const bool enable_validation_layers = true;
#endif

// Hardcoded validation layers
static const char* validation_layers[] = {
	"VK_LAYER_KHRONOS_validation"
};
static const uint32_t validation_layer_count = 1;

static lise_vulkan_context vulkan_context;

static bool check_validation_layer_support();

bool lise_vulkan_initialize(const char* application_name)
{
	if (enable_validation_layers && !check_validation_layer_support())
	{
		LFATAL("One or more requested validation layers do not exist.");
		return false;
	}

	// Vulkan Instance 
	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.apiVersion = VK_API_VERSION_1_3;
	app_info.pApplicationName = application_name;
	app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	app_info.pEngineName = "Lipin Sock Engine";
	app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);

	VkInstanceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;

	// Instance Extensions
	uint32_t platform_extension_count;
	const char** platform_extensions;

	platform_extensions = lise_platform_get_required_instance_extensions(&platform_extension_count);

	create_info.enabledExtensionCount = platform_extension_count;
	create_info.ppEnabledExtensionNames = platform_extensions;

	// Validation layers
	if (enable_validation_layers)
	{
		create_info.enabledLayerCount = validation_layer_count;
		create_info.ppEnabledLayerNames = validation_layers;
	}
	else
	{
		create_info.enabledLayerCount = 0;
	}

	// Create vulkan instance
	if (vkCreateInstance(&create_info, NULL, &vulkan_context.instance))
	{
		LFATAL("Failed to create Vulkan instance.");
		return false;
	}

	// Create vulkan surface
	if (!lise_vulkan_platform_create_vulkan_surface(vulkan_context.instance,
		&vulkan_context.surface))
	{
		LFATAL("Failed to create vulkan surface.");
		return false;
	}

	return true;
}

void lise_vulkan_shutdown()
{
	
	vkDestroyInstance(vulkan_context.instance, NULL);
}

// Static helper functions

static bool check_validation_layer_support()
{
	// Get available validation layers
	uint32_t available_layer_count;
	vkEnumerateInstanceLayerProperties(&available_layer_count, NULL);

	VkLayerProperties* available_layers = malloc(sizeof(VkLayerProperties) * available_layer_count);
	vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers);

	for (uint32_t i = 0; i < validation_layer_count; i++)
	{
		bool layer_found = false;

		// Loop through available layers and check if our layers are among them
		for (uint32_t j = 0; j < available_layer_count; j++)
		{
			if (strcmp(validation_layers[i], available_layers[j].layerName) == 0)
			{
				layer_found = true;
				break;
			}
		}

		if (!layer_found)
		{
			free(available_layers);
			return false;
		}
	}

	free(available_layers);
	return true;
}
