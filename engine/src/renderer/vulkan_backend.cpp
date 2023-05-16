#include "renderer/vulkan_backend.hpp"

#include <memory>
#include <cstring>

#include "core/logger.hpp"
#include "platform/platform.hpp"

namespace lise
{

// State variables.
static VkInstance instance;

static VkSurfaceKHR surface;

static Device* device;

static Swapchain* swapchain;
static RenderPass* render_pass;

static std::vector<CommandBuffer> graphics_command_buffers;

static std::vector<VkSemaphore> image_available_semaphores;

static std::vector<VkSemaphore> queue_complete_semaphores;

static uint32_t in_flight_fence_count;
static std::vector<Fence> in_flight_fences;

static std::vector<Fence*> images_in_flight;

static uint32_t current_image_index;

static VulkanBuffer* object_vertex_buffer;
static VulkanBuffer* object_index_buffer;

#ifdef NDEBUG
	static constexpr bool enable_validation_layers = false;
#else
	static constexpr bool enable_validation_layers = true;
#endif

// Hardcoded validation layers
static constexpr const char* validation_layers[] = {
	"VK_LAYER_KHRONOS_validation"
};
static constexpr uint32_t validation_layer_count = 1;

// Hardcoded device extensions
static constexpr const char* device_extensions[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
static constexpr uint32_t device_extension_count = 1;

// Static helper functions.
static bool check_validation_layer_support();


bool vulkan_initialize(const char* application_name)
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

	platform_extensions = platform_get_required_instance_extensions(&platform_extension_count);

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
	if (vkCreateInstance(&create_info, NULL, &instance) != VK_SUCCESS)
	{
		LFATAL("Failed to create Vulkan instance.");
		return false;
	}
}

void vulkan_shutdown()
{
	vkDeviceWaitIdle(*device);

	LINFO("Successfully shut down the vulkan backend.");
}

bool vulkan_begin_frame(float delta_time)
{

}

bool vulkan_end_frame(float delta_time)
{

}

vector2i vulkan_get_framebuffer_size()
{

}

// TODO: temp hack
LAPI void vulkan_set_view_matrix_temp(const mat4x4& view)
{

}

// Static helper functions.
static bool check_validation_layer_support()
{
	// Get available validation layers.
	uint32_t available_layer_count;
	vkEnumerateInstanceLayerProperties(&available_layer_count, NULL);

	std::unique_ptr<VkLayerProperties[]> available_layers =
		std::make_unique<VkLayerProperties[]>(available_layer_count);
	vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers.get());

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
			return false;
		}
	}

	return true;
}

}
