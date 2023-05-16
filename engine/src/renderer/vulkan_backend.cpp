#include "renderer/vulkan_backend.hpp"

#include <memory>
#include <cstring>

#include "core/logger.hpp"
#include "platform/platform.hpp"
#include "renderer/vulkan_platform.hpp"

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
static const char* validation_layers[] = {
	"VK_LAYER_KHRONOS_validation"
};
static uint32_t validation_layer_count = 1;

// Hardcoded device extensions
static const char* device_extensions[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
static uint32_t device_extension_count = 1;

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

	// Create vulkan surface
	if (!vulkan_platform_create_vulkan_surface(instance, &surface))
	{
		LFATAL("Failed to create vulkan surface.");
		return false;
	}


	// Create the device
	try
	{
		device = new Device(
			instance,
			device_extensions,
			device_extension_count,
			validation_layers,
			validation_layer_count,
			surface
		);
	}
	catch (std::exception e)
	{
		LFATAL("Failed to create a logical device.");
		return false;
	}

	// Get swapchain info
	SwapchainInfo swapchain_info = Swapchain::query_info(*device, surface);
	
	// Create the render pass
	try
	{
		render_pass = new RenderPass(
			*device,
			swapchain_info.image_format.format,
			swapchain_info.depth_format,
			vector2ui { 0, 0 },
			vector2ui { swapchain_info.swapchain_extent.width, swapchain_info.swapchain_extent.height },
			vector4f { 0.4f, 0.5f, 0.6f, 0.0f },
			1.0f,
			0
		);
	}
	catch (std::exception e)
	{
		LFATAL("Failed to create render pass.");
		return false;
	}

	// Create the swapchain
	try
	{
		swapchain = new Swapchain(
			*device,
			surface,
			swapchain_info,
			*render_pass
		);
	}
	catch (std::exception e)
	{
		LFATAL("Failed to create the swapchain.");
		return false;
	}

	return true;
}

void vulkan_shutdown()
{
	vkDeviceWaitIdle(*device);

	delete render_pass;

	delete swapchain;

	delete device;

	vkDestroySurfaceKHR(instance, surface, nullptr);

	vkDestroyInstance(instance, nullptr);

	LINFO("Successfully shut down the vulkan backend.");
}

bool vulkan_begin_frame(float delta_time)
{

	return true;
}

bool vulkan_end_frame(float delta_time)
{

	return true;
}

vector2ui vulkan_get_framebuffer_size()
{
	return vector2ui {
		swapchain->get_swapchain_info().swapchain_extent.width,
		swapchain->get_swapchain_info().swapchain_extent.height
	};
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
