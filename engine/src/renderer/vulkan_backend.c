#include "renderer/vulkan_backend.h"

#include <string.h>
#include <stdlib.h>

#include "core/event.h"
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

// Hardcoded device extensions
static const char* device_extensions[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
static const uint32_t device_extension_count = 1;

static lise_vulkan_context vulkan_context;

// Static helper functions
static bool check_validation_layer_support();
static void create_command_buffers();
static bool recreate_swapchain();

bool lise_vulkan_initialize(lise_vec2i window_extent, const char* application_name)
{
	if (enable_validation_layers && !check_validation_layer_support())
	{
		LFATAL("One or more requested validation layers do not exist.");
		return false;
	}

	vulkan_context.framebuffer_width = window_extent.x;
	vulkan_context.framebuffer_height = window_extent.y;

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
	if (vkCreateInstance(&create_info, NULL, &vulkan_context.instance) != VK_SUCCESS)
	{
		LFATAL("Failed to create Vulkan instance.");
		return false;
	}

	// Create vulkan surface
	if (!lise_vulkan_platform_create_vulkan_surface(vulkan_context.instance,
		&vulkan_context.surface
	))
	{
		LFATAL("Failed to create vulkan surface.");
		return false;
	}

	// Create the device
	if (!lise_device_create(
			vulkan_context.instance,
			device_extensions,
			device_extension_count,
			validation_layers,
			validation_layer_count,
			vulkan_context.surface,
			&vulkan_context.device
	))
	{
		LFATAL("Failed to create a logical device.");
		return false;
	}

	// Get swapchain info
	lise_swapchain_info swapchain_info = lise_swapchain_query_info(
		&vulkan_context.device,
		vulkan_context.surface
	);
	
	// Create the render pass
	if (!lise_render_pass_create(
		&vulkan_context.device,
		swapchain_info.image_format.format,
		swapchain_info.depth_format,
		0, 0, swapchain_info.swapchain_extent.width, swapchain_info.swapchain_extent.height,
		0.0f, 0.0f, 0.0f, 0.0f,
		1.0f,
		0,
		&vulkan_context.render_pass
	))
	{
		LFATAL("Failed to create render pass.");
		return false;
	}

	// Create the swapchain
	if (!lise_swapchain_create(
		&vulkan_context.device,
		vulkan_context.surface,
		swapchain_info,
		&vulkan_context.render_pass,
		&vulkan_context.swapchain
	))
	{
		LFATAL("Failed to create the swapchain.");
		return false;
	}

	// Create the graphics command buffers
	create_command_buffers();

	// Create sync objects
	vulkan_context.image_available_semaphores =
		malloc(sizeof(VkSemaphore) * vulkan_context.swapchain.max_frames_in_flight);
	
	vulkan_context.queue_complete_semaphores =
		malloc(sizeof(VkSemaphore) * vulkan_context.swapchain.max_frames_in_flight);

	vulkan_context.in_flight_fences = malloc(sizeof(lise_fence) * vulkan_context.swapchain.max_frames_in_flight);

	for (uint32_t i = 0; i < vulkan_context.swapchain.max_frames_in_flight; i++)
	{
		VkSemaphoreCreateInfo semaphore_ci = {};
		semaphore_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		vkCreateSemaphore(
			vulkan_context.device.logical_device,
			&semaphore_ci,
			NULL,
			&vulkan_context.image_available_semaphores[i]
		);

		vkCreateSemaphore(
			vulkan_context.device.logical_device,
			&semaphore_ci,
			NULL,
			&vulkan_context.queue_complete_semaphores[i]
		);

		lise_fence_create(vulkan_context.device.logical_device, true, &vulkan_context.in_flight_fences[i]);
	}

	vulkan_context.images_in_flight = calloc(vulkan_context.swapchain.image_count, sizeof(lise_fence*));

	LINFO("Successfully initialized the vulkan backend.");

	return true;
}

void lise_vulkan_shutdown()
{
	vkDeviceWaitIdle(vulkan_context.device.logical_device);

	for (uint32_t i = 0; i < vulkan_context.swapchain.max_frames_in_flight; i++)
	{
		if (vulkan_context.image_available_semaphores[i])
		{
			vkDestroySemaphore(
				vulkan_context.device.logical_device, 
				vulkan_context.image_available_semaphores[i],
				NULL
			);
		}

		if (vulkan_context.queue_complete_semaphores[i])
		{
			vkDestroySemaphore(
				vulkan_context.device.logical_device, 
				vulkan_context.queue_complete_semaphores[i],
				NULL
			);
		}

		lise_fence_destroy(vulkan_context.device.logical_device, &vulkan_context.in_flight_fences[i]);
	}
	free(vulkan_context.image_available_semaphores);
	free(vulkan_context.queue_complete_semaphores);
	free(vulkan_context.in_flight_fences);

	free(vulkan_context.images_in_flight);

	for (uint32_t i = 0; i < vulkan_context.swapchain.image_count; i++)
	{
		if (vulkan_context.graphics_command_buffers->handle)
		{
			lise_command_buffer_free(
				vulkan_context.device.logical_device,
				vulkan_context.device.graphics_command_pool,
				&vulkan_context.graphics_command_buffers[i]
			);
		}
	}
	free(vulkan_context.graphics_command_buffers);

	lise_render_pass_destroy(vulkan_context.device.logical_device, &vulkan_context.render_pass);

	lise_swapchain_destroy(vulkan_context.device.logical_device, &vulkan_context.swapchain);

	lise_device_destroy(&vulkan_context.device);

	vkDestroySurfaceKHR(vulkan_context.instance, vulkan_context.surface, NULL);
	
	vkDestroyInstance(vulkan_context.instance, NULL);

	LINFO("Successfully shut the vulkan backend down.");
}

bool lise_vulkan_begin_frame(float delta_time)
{
	if (vulkan_context.swapchain.swapchain_out_of_date)
	{
		recreate_swapchain();
		return lise_vulkan_begin_frame(delta_time);
	}

	// Wait for the current frame
	if (!lise_fence_wait(
		vulkan_context.device.logical_device,
		&vulkan_context.in_flight_fences[vulkan_context.swapchain.current_frame],
		UINT64_MAX
	))
	{
		LWARN("Failed to wait on an in-flight fence.");
		return false;
	}

	// Acquire next image in swapchain
	if (!lise_swapchain_acquire_next_image_index(
		&vulkan_context.device,
		&vulkan_context.swapchain,
		UINT64_MAX,
		vulkan_context.image_available_semaphores[vulkan_context.swapchain.current_frame],
		NULL,
		&vulkan_context.current_image_index
	))
	{
		LWARN("Failed to acquire next swapchain image");
		return false;
	}

	// Begin command buffer
	lise_command_buffer* command_buffer = &vulkan_context.graphics_command_buffers[vulkan_context.current_image_index];

	lise_command_buffer_reset(command_buffer);
	lise_command_buffer_begin(command_buffer, false, false, false);

	// Dynamic state
	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = (float) vulkan_context.framebuffer_height;
	viewport.width = (float) vulkan_context.framebuffer_width;
	viewport.height = -(float) vulkan_context.framebuffer_height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Scissor
	VkRect2D scissor;
	scissor.offset.x = scissor.offset.y = 0;
	scissor.extent.width = vulkan_context.framebuffer_width;
	scissor.extent.height = vulkan_context.framebuffer_height;

	vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
	vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);

	lise_render_pass_begin(
		command_buffer,
		&vulkan_context.render_pass,
		vulkan_context.swapchain.framebuffers[vulkan_context.current_image_index].handle
	);

	return true;
}

bool lise_vulkan_end_frame(float delta_time)
{
	lise_command_buffer* command_buffer = &vulkan_context.graphics_command_buffers[vulkan_context.current_image_index];

	// End render pass
	lise_render_pass_end(command_buffer, &vulkan_context.render_pass);

	lise_command_buffer_end(command_buffer);

	// Wait if a previous frame is still using this image
	if (vulkan_context.images_in_flight[vulkan_context.current_image_index] != NULL)
	{
		lise_fence_wait(
			vulkan_context.device.logical_device,
			vulkan_context.images_in_flight[vulkan_context.current_image_index],
			UINT64_MAX
		);
	}

	// Mark fence as being in use by this image
	vulkan_context.images_in_flight[vulkan_context.current_image_index] = 
		&vulkan_context.in_flight_fences[vulkan_context.swapchain.current_frame];

	// Reset the fence
	lise_fence_reset(
		vulkan_context.device.logical_device,
		&vulkan_context.in_flight_fences[vulkan_context.swapchain.current_frame]
	);

	// Submit queue
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer->handle;

	// Semaphores to be signaled when the queue is completed
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &vulkan_context.queue_complete_semaphores[vulkan_context.swapchain.current_frame];

	// Wait before queue execution until image is available (image available semaphore is signaled)
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &vulkan_context.image_available_semaphores[vulkan_context.swapchain.current_frame];

	// Wait destination
	VkPipelineStageFlags stage_flags[1] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};

	submit_info.pWaitDstStageMask = stage_flags;

	if (vkQueueSubmit(
		vulkan_context.device.graphics_queue,
		1,
		&submit_info,
		vulkan_context.in_flight_fences[vulkan_context.swapchain.current_frame].handle) != VK_SUCCESS
	)
	{
		LERROR("Failed to submit queue.");
		return false;
	}

	lise_command_buffer_update_submitted(command_buffer);

	// Give images back to the swapchain
	if (!lise_swapchain_present(
		&vulkan_context.device,
		&vulkan_context.swapchain,
		vulkan_context.queue_complete_semaphores[vulkan_context.swapchain.current_frame],
		vulkan_context.current_image_index
		) && !vulkan_context.swapchain.swapchain_out_of_date
	)
	{
		// Swapchain is not out of date, so the return value indicates a failure.
		LFATAL("Failed to present swap chain image.");
		return false;
	}

	return true;
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

void create_command_buffers()
{
	if (!vulkan_context.graphics_command_buffers)
	{
		LDEBUG("Creating initial command buffers");

		// Reserve memory for the command buffers. There are as many command buffers as there are drawable surfaces in
		// the swapchain.
		vulkan_context.graphics_command_buffers = calloc(
			vulkan_context.swapchain.image_count, 
			sizeof(lise_command_buffer)
		);
	}

	for (uint32_t i = 0; i < vulkan_context.swapchain.image_count; i++)
	{
		// Free buffer if exists
		if (vulkan_context.graphics_command_buffers[i].handle)
		{
			lise_command_buffer_free(
				vulkan_context.device.logical_device,
				vulkan_context.device.graphics_command_pool,
				&vulkan_context.graphics_command_buffers[i]
			);
		}

		memset(&vulkan_context.graphics_command_buffers[i], 0, sizeof(lise_command_buffer));

		lise_command_buffer_allocate(
			vulkan_context.device.logical_device,
			vulkan_context.device.graphics_command_pool,
			true,
			&vulkan_context.graphics_command_buffers[i]
		);
	}
}

static bool recreate_swapchain()
{
	// Wait for the device to idle
	vkDeviceWaitIdle(vulkan_context.device.logical_device);

	memset(vulkan_context.images_in_flight, 0, sizeof(lise_fence*) * vulkan_context.swapchain.image_count);

	lise_swapchain_info new_info = lise_swapchain_query_info(
		&vulkan_context.device,
		vulkan_context.surface
	);

	lise_render_pass_recreate(
		&vulkan_context.device,
		new_info.image_format.format,
		new_info.depth_format,
		0, 0, new_info.swapchain_extent.width, new_info.swapchain_extent.height,
		1.0f, 0.0f, 0.0f, 0.0f,
		1.0f,
		0,
		&vulkan_context.render_pass
	);

	lise_swapchain_recreate(
		&vulkan_context.device,
		vulkan_context.surface,
		new_info,
		&vulkan_context.render_pass,
		&vulkan_context.swapchain
	);
}
