#include "renderer/vulkan_backend.h"

#include <string.h>
#include <stdlib.h>

#include "core/event.h"
#include "core/logger.h"
#include "platform/platform.h"
#include "renderer/vulkan_platform.h"
#include "math/vertex.h"
#include "math/quat.h"
#include "math/math.h"
#include "loader/obj_loader.h"

// Renderer subsystems.
#include "renderer/system/texture_system.h"
#include "renderer/system/shader_system.h"

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

// TODO: Temp function
void upload_data_range(lise_vulkan_buffer* buffer, uint64_t offset, uint64_t size, void* data)
{
	// Create a host-visible staging buffer to upload to. Mark it as the source of the transfer.
	VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	lise_vulkan_buffer staging;
	lise_vulkan_buffer_create(
		vulkan_context.device.logical_device,
		vulkan_context.device.physical_device_memory_properties,
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		flags,
		true,
		&staging
	);

	// Load the data into the staging buffer.
	lise_vulkan_buffer_load_data(vulkan_context.device.logical_device, &staging, 0, size, 0, data);

	// Perform the copy from staging to the device local buffer.
	lise_vulkan_buffer_copy_to(
		vulkan_context.device.logical_device,
		vulkan_context.device.graphics_command_pool,
		0,
		vulkan_context.device.graphics_queue,
		staging.handle,
		0,
		buffer->handle,
		offset,
		size
	);

	// Clean up the staging buffer.
	lise_vulkan_buffer_destroy(vulkan_context.device.logical_device, &staging);
}

// TODO: temp statics
static lise_mat4x4 view_matrix = LMAT4X4_IDENTITY;
static lise_shader_instance test_object;
static lise_texture temp_texture;
static lise_obj test_model;

static lise_shader* obj_shader;

typedef struct global_ubo
{
	lise_mat4x4 projection;
	lise_mat4x4 view;
} global_ubo;

typedef struct instance_ubo
{
	lise_vec4 diffuse_color;
} instance_ubo;

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
		0.4f, 0.5f, 0.6f, 0.0f,
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

	if (!lise_texture_system_initialize(&vulkan_context.device))
	{
		LFATAL("Failed to initialize the vulkan renderer texture subsystem.");
		return false;
	}

	if (!lise_shader_system_initialize(
		&vulkan_context.device,
		&vulkan_context.render_pass,
		&vulkan_context.swapchain.swapchain_info.swapchain_extent.width,
		&vulkan_context.swapchain.swapchain_info.swapchain_extent.height,
		&vulkan_context.swapchain.image_count
	))
	{
		LFATAL("Failed to initialize the vulkan renderer shader subsystem.");
		return false;
	}

	// Load default shaders.
	lise_shader_system_load("assets/shaders/builtin.object_shader.scfg", &obj_shader);

	// -------- TEMP
	// Test obj.
	if (!lise_obj_load("assets/models/obj/test_cube.obj", &test_model))
	{
		LFATAL("Failed to load test_cube.obj.");
		return false;
	}

	// Create the vertex and index buffers
	VkMemoryPropertyFlagBits mem_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	if (!lise_vulkan_buffer_create(
		vulkan_context.device.logical_device,
		vulkan_context.device.physical_device_memory_properties,
		sizeof(lise_vertex) * test_model.meshes[0].vertex_count,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		mem_flags,
		true,
		&vulkan_context.object_vertex_buffer
	))
	{
		LFATAL("Failed to create the vertex buffer.");
	}
	
	if (!lise_vulkan_buffer_create(
		vulkan_context.device.logical_device,
		vulkan_context.device.physical_device_memory_properties,
		sizeof(uint32_t) * test_model.meshes[0].index_count,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		mem_flags,
		true,
		&vulkan_context.object_index_buffer
	))
	{
		LFATAL("Failed to create the index buffer.");
	}

	const uint32_t vert_count = 4;
	lise_vertex verts[4];

	memset(verts, 0, sizeof(lise_vertex) * vert_count);

	verts[0].position.x = -0.75;
	verts[0].position.y = -0.5;
	verts[0].tex_coord.x = 0.0f;
	verts[0].tex_coord.y = 0.0f;
	verts[0].normal.x = 0.0f;
	verts[0].normal.y = 0.0f;

	verts[1].position.y = 0.5;
	verts[1].position.x = 0.75;
	verts[1].tex_coord.x = 1.0f;
	verts[1].tex_coord.y = 1.0f;
	verts[1].normal.x = 0.0f;
	verts[1].normal.y = 0.0f;

	verts[2].position.x = -0.5;
	verts[2].position.y = 0.5;
	verts[2].tex_coord.x = 0.0f;
	verts[2].tex_coord.y = 1.0f;
	verts[2].normal.x = 0.0f;
	verts[2].normal.y = 0.0f;

	verts[3].position.x = 0.5;
	verts[3].position.y = -0.5;
	verts[3].tex_coord.x = 1.0f;
	verts[3].tex_coord.y = 0.0f;
	verts[3].normal.x = 0.0f;
	verts[3].normal.y = 0.0f;

	const uint32_t index_count = 6;
	uint32_t indices[6] = {0, 1, 2, 0, 3, 1};

//	upload_data_range(&vulkan_context.object_vertex_buffer, 0, sizeof(lise_vertex) * vert_count, verts);
//	upload_data_range(&vulkan_context.object_index_buffer, 0, sizeof(uint32_t) * index_count, indices);

	upload_data_range(&vulkan_context.object_vertex_buffer, 0, sizeof(lise_vertex) * test_model.meshes[0].vertex_count, test_model.meshes[0].vertices);
	upload_data_range(&vulkan_context.object_index_buffer, 0, sizeof(uint32_t) * test_model.meshes[0].index_count, test_model.meshes[0].indices);

	lise_texture_system_load(&vulkan_context.device, "assets/texture/test_texture.png", &temp_texture);

	lise_shader_allocate_instance(vulkan_context.device.logical_device, obj_shader, &test_object);

	lise_shader_set_instance_sampler(vulkan_context.device.logical_device, obj_shader, 0, &temp_texture, &test_object);

	// --------- ENDTEMP

	LINFO("Successfully initialized the vulkan backend.");

	return true;
}

void lise_vulkan_shutdown()
{
	vkDeviceWaitIdle(vulkan_context.device.logical_device);

	lise_vulkan_buffer_destroy(vulkan_context.device.logical_device, &vulkan_context.object_index_buffer);
	lise_vulkan_buffer_destroy(vulkan_context.device.logical_device, &vulkan_context.object_vertex_buffer);

	lise_shader_system_shutdown();

	lise_texture_system_shutdown(vulkan_context.device.logical_device);

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

	LINFO("Successfully shut down the vulkan backend.");
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
	viewport.y = (float) vulkan_context.swapchain.swapchain_info.swapchain_extent.height;
	viewport.width = (float) vulkan_context.swapchain.swapchain_info.swapchain_extent.width;
	viewport.height = -(float) vulkan_context.swapchain.swapchain_info.swapchain_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Scissor
	VkRect2D scissor;
	scissor.offset.x = scissor.offset.y = 0;
	scissor.extent.width = vulkan_context.swapchain.swapchain_info.swapchain_extent.width;
	scissor.extent.height = vulkan_context.swapchain.swapchain_info.swapchain_extent.height;

	vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
	vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);

	lise_render_pass_begin(
		command_buffer,
		&vulkan_context.render_pass,
		vulkan_context.swapchain.framebuffers[vulkan_context.current_image_index].handle
	);

	// -------- TEMP
	lise_vec2i framebuffer_size = lise_vulkan_get_framebuffer_size();

	global_ubo gubo = {};
	gubo.projection = 
		lise_mat4x4_perspective(LQUARTER_PI, (float) framebuffer_size.x / (float) framebuffer_size.y, 0.1f, 1000.0f);
	
	gubo.view = view_matrix;

	lise_mat4x4 model = LMAT4X4_IDENTITY;

	vkCmdPushConstants(command_buffer->handle, obj_shader->pipeline.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, 64, &model);

	lise_shader_set_global_ubo(vulkan_context.device.logical_device, obj_shader, &gubo);

	lise_shader_update_global_uniforms(vulkan_context.device.logical_device, obj_shader, vulkan_context.current_image_index, &test_object);

	// Bind uniform and sampler object descriptors.
	static float accumulator = 0.0f;
	accumulator += delta_time;
	float s = (lsin(accumulator) + 1.0f) / 2.0f;

	instance_ubo iubo = {};

	iubo.diffuse_color = (lise_vec4) { s, s, s, 1.0f };

	lise_shader_set_instance_ubo(vulkan_context.device.logical_device, obj_shader, &iubo, &test_object);
	lise_shader_update_instance_ubo(vulkan_context.device.logical_device, obj_shader, vulkan_context.current_image_index, &test_object);

	vkCmdBindDescriptorSets(
		command_buffer->handle,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		obj_shader->pipeline.pipeline_layout,
		1,
		1,
		&test_object.descriptor_sets[vulkan_context.current_image_index],
		0,
		0
	);

//	lise_object_shader_use(vulkan_context.current_image_index, command_buffer->handle, &vulkan_context.object_shader);
	lise_shader_use(obj_shader, command_buffer->handle, vulkan_context.current_image_index);

	// Bind vertex
	VkDeviceSize offsets[1] = {0};
	vkCmdBindVertexBuffers(command_buffer->handle, 0, 1, &vulkan_context.object_vertex_buffer.handle, (VkDeviceSize*) offsets);

	// Bind index
	vkCmdBindIndexBuffer(command_buffer->handle, vulkan_context.object_index_buffer.handle, 0, VK_INDEX_TYPE_UINT32);

	// Issue draw call
	vkCmdDrawIndexed(command_buffer->handle, test_model.meshes[0].index_count, 1, 0, 0, 0);

	// -------- ENDTEMP

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
		0.4f, 0.5f, 0.6f, 0.0f,
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

lise_vec2i lise_vulkan_get_framebuffer_size()
{
	return (lise_vec2i) 
	{
		vulkan_context.swapchain.swapchain_info.swapchain_extent.width,
		vulkan_context.swapchain.swapchain_info.swapchain_extent.height
	};
}

// TODO: temp hack
void lise_vulkan_set_view_matrix_temp(lise_mat4x4 view)
{
	view_matrix = view;
}
