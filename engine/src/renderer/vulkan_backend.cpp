#include "renderer/vulkan_backend.hpp"

#include <memory>
#include <cstring>

#include "core/logger.hpp"
#include "platform/platform.hpp"
#include "renderer/vulkan_platform.hpp"
#include "renderer/resource/model.hpp"

#include "renderer/system/texture_system.hpp"
#include "renderer/system/shader_system.hpp"

namespace lise
{

// State variables.
static VkInstance instance;

static VkSurfaceKHR surface;

static Device* device;

static Swapchain* swapchain;
static RenderPass* world_render_pass;
static RenderPass* ui_render_pass;

static std::vector<CommandBuffer> graphics_command_buffers;

static std::vector<VkSemaphore> image_available_semaphores;

static std::vector<VkSemaphore> queue_complete_semaphores;

static uint32_t in_flight_fence_count;
static std::vector<Fence> in_flight_fences;

static std::vector<Fence*> images_in_flight;

static uint32_t current_image_index;

static Shader* object_shader;

static Shader* ui_shader;

static std::vector<Framebuffer> world_framebuffers;

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
static void create_command_buffers();
static bool recreate_swapchain();

// TODO: temp statics
static mat4x4 view_matrix = LMAT4X4_IDENTITY;
static Texture* temp_texture;

//static Model test_model;
static Model* car_model;

struct GlobalUBO
{
	mat4x4 projection;
	mat4x4 view;
};

struct InstanceUBO
{
	vector4f diffuse_color;
};

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
		world_render_pass = new RenderPass(
			*device,
			swapchain_info.image_format.format,
			swapchain_info.depth_format,
			vector2ui { 0, 0 },
			vector2ui { swapchain_info.swapchain_extent.width, swapchain_info.swapchain_extent.height },
			vector4f { 0.4f, 0.5f, 0.6f, 0.0f },
			1.0f,
			0,
			RenderPassClearFlag::COLOR_BUFFER_FLAG | RenderPassClearFlag::DEPTH_BUFFER_FLAG |
				RenderPassClearFlag::STENCIL_BUFFER_FLAG,
			false,
			true
		);

		ui_render_pass = new RenderPass(
			*device,
			swapchain_info.image_format.format,
			swapchain_info.depth_format,
			vector2ui { 0, 0 },
			vector2ui { swapchain_info.swapchain_extent.width, swapchain_info.swapchain_extent.height },
			vector4f { 0.0f, 0.0f, 0.0f, 0.0f },
			1.0f,
			0,
			RenderPassClearFlag::NONE_FLAG,
			true,
			false
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
			*ui_render_pass
		);
	}
	catch (std::exception e)
	{
		LFATAL("Failed to create the swapchain.");
		return false;
	}

	// Create world framebuffers.
	world_framebuffers.reserve(swapchain->get_images().size());

	for (size_t i = 0; i < swapchain->get_images().size(); i++)
	{
		VkImageView attachments[2] = {
			swapchain->get_image_views()[i],
			swapchain->get_depth_attachments()[i].get_image_view()
		};

		world_framebuffers.emplace_back(
			*device,
			*world_render_pass,
			vulkan_get_framebuffer_size(),
			2,
			attachments
		);
	}
	
	// Create command buffers.
	create_command_buffers();

	// Create sync objects
	uint8_t max_frames_in_flight = swapchain->get_max_frames_in_flight();
	
	image_available_semaphores.resize(max_frames_in_flight);
	
	queue_complete_semaphores.resize(max_frames_in_flight);

	in_flight_fences.reserve(max_frames_in_flight);

	for (uint32_t i = 0; i < max_frames_in_flight; i++)
	{
		VkSemaphoreCreateInfo semaphore_ci = {};
		semaphore_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		vkCreateSemaphore(
			*device,
			&semaphore_ci,
			NULL,
			&image_available_semaphores[i]
		);

		vkCreateSemaphore(
			*device,
			&semaphore_ci,
			NULL,
			&queue_complete_semaphores[i]
		);

		in_flight_fences.emplace_back(*device, true); 
	}

	// Preallocate the in flight images and set them to nullptr;
	images_in_flight.resize(swapchain->get_images().size(), nullptr);

	if (!texture_system_initialize(*device))
	{
		LFATAL("Failed to initialize the vulkan renderer texture subsystem.");
		return false;
	}

	if (!shader_system_initialize(
		*device,
		*swapchain
	))
	{
		LFATAL("Failed to initialize the vulkan renderer shader subsystem.");
		return false;
	}

	// Load default shaders.
	object_shader = shader_system_load("assets/shaders/builtin.object_shader.scfg", *world_render_pass);
	ui_shader = shader_system_load("assets/shaders/builtin.ui_shader.scfg", *ui_render_pass);

	if (object_shader == nullptr)
	{
		LFATAL("Failed to load the object shader.");
		
		return false;
	}

	if (ui_shader == nullptr)
	{
		LFATAL("Failed to load the ui shader.");
		
		return false;
	}

	// TEMP:
	
	std::optional<Obj> car_obj = Obj::load("assets/models/obj/car.obj");

	if (!car_obj)
	{
		LFATAL("Failed to load car obj file.");

		return false;
	}

	car_model = new Model(device, object_shader, *car_obj);
	car_model->get_transform().set_position(0, 0, -10);

	return true;
}

void vulkan_shutdown()
{
	vkDeviceWaitIdle(*device);

	delete car_model;

	shader_system_shutdown();

	texture_system_shutdown(*device);

	for (size_t i = 0; i < image_available_semaphores.size(); i++)
	{
		vkDestroySemaphore(*device, image_available_semaphores[i], nullptr);
		vkDestroySemaphore(*device, queue_complete_semaphores[i], nullptr);
	}

	in_flight_fences.clear();

	graphics_command_buffers.clear();

	world_framebuffers.clear();

	delete ui_render_pass;

	delete world_render_pass;

	delete swapchain;

	delete device;

	vkDestroySurfaceKHR(instance, surface, nullptr);

	vkDestroyInstance(instance, nullptr);

	LINFO("Successfully shut down the vulkan backend.");
}

bool vulkan_begin_frame(float delta_time)
{
	if (swapchain->is_swapchain_out_of_date())
	{
		recreate_swapchain();
		return vulkan_begin_frame(delta_time);
	}

	uint8_t current_frame = swapchain->get_current_frame();

	// Wait for the current frame
	if (!in_flight_fences[current_frame].wait())
	{
		LWARN("Failed to wait on an in-flight fence.");
		return false;
	}

	// Acquire next image in swapchain
	if (!swapchain->acquire_next_image_index(
		UINT64_MAX,
		image_available_semaphores[current_frame],
		NULL,
		current_image_index
	))
	{
		LWARN("Failed to acquire next swapchain image");
		return false;
	}

	// Begin command buffer
	CommandBuffer& command_buffer = graphics_command_buffers[current_frame];
	command_buffer.reset();
	command_buffer.begin(false, false, false);

	// Dynamic state
	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = (float) swapchain->get_swapchain_info().swapchain_extent.height;
	viewport.width = (float) swapchain->get_swapchain_info().swapchain_extent.width;
	viewport.height = -(float) swapchain->get_swapchain_info().swapchain_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Scissor
	VkRect2D scissor;
	scissor.offset.x = scissor.offset.y = 0;
	scissor.extent.width = swapchain->get_swapchain_info().swapchain_extent.width;
	scissor.extent.height = swapchain->get_swapchain_info().swapchain_extent.height;

	vkCmdSetViewport(command_buffer, 0, 1, &viewport);
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

	world_render_pass->begin(command_buffer, world_framebuffers[current_image_index]);

	// -------- TEMP
	vector2ui framebuffer_size = vulkan_get_framebuffer_size();

	GlobalUBO gubo = {};
	gubo.projection =
		mat4x4::perspective(LQUARTER_PI, (float) framebuffer_size.x / (float) framebuffer_size.y, 0.1f, 1000.0f);
	
	gubo.view = view_matrix;

	object_shader->set_global_ubo(&gubo);
	object_shader->update_global_uniforms(current_frame);

	//test_model.transform.rotation.y += LQUARTER_PI * delta_time;
	//lise_transform_update(&test_model.transform);
	car_model->get_transform().set_rotation(car_model->get_transform().get_rotation() + vector3f {0, LQUARTER_PI * delta_time});
	
	car_model->draw(command_buffer, current_frame);

	//lise_model_draw(&test_model, vulkan_context.device.logical_device, command_buffer->handle, vulkan_context.current_image_index);
	//
	//lise_model_draw(&car_model, vulkan_context.device.logical_device, command_buffer->handle, vulkan_context.current_image_index);

	// -------- ENDTEMP

	return true;
}

bool vulkan_end_frame(float delta_time)
{
	uint8_t current_frame = swapchain->get_current_frame();
	CommandBuffer& command_buffer = graphics_command_buffers[current_frame];

	// End render pass.
	world_render_pass->end(command_buffer);

	ui_render_pass->begin(command_buffer, swapchain->get_framebuffers()[current_image_index]);

	ui_render_pass->end(command_buffer);

	command_buffer.end();

	// Wait if a previous frame is still using this image.
	if (images_in_flight[current_frame] != NULL)
	{
		images_in_flight[current_frame]->wait();
	}

	// Mark fence as being in use by this image.
	images_in_flight[current_frame] = &in_flight_fences[current_frame];

	// Reset the fence
	images_in_flight[current_frame]->reset();

	// Submit queue
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &((const VkCommandBuffer&) command_buffer);

	// Semaphores to be signaled when the queue is completed
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &queue_complete_semaphores[current_frame];

	// Wait before queue execution until image is available (image available semaphore is signaled)
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &image_available_semaphores[current_frame];

	// Wait destination
	VkPipelineStageFlags stage_flags[1] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};

	submit_info.pWaitDstStageMask = stage_flags;

	if (vkQueueSubmit(
		device->get_graphics_queue(),
		1,
		&submit_info,
		in_flight_fences[current_frame]) != VK_SUCCESS
	)
	{
		LERROR("Failed to submit queue.");

		return false;
	}

	command_buffer.set_state(CommandBufferState::SUBMITTED);

	// Give images back to the swapchain
	if (!swapchain->present(
		queue_complete_semaphores[swapchain->get_current_frame()],
		current_image_index
		) && !swapchain->is_swapchain_out_of_date()
	)
	{
		// Swapchain is not out of date, so the return value indicates a failure.
		LFATAL("Failed to present swap chain image.");

		return false;
	}

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
	view_matrix = view;
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

void create_command_buffers()
{
	LDEBUG("Creating initial command buffers");

	// Clear old command buffers.
	graphics_command_buffers.clear();

	graphics_command_buffers.reserve(swapchain->get_max_frames_in_flight());

	for (uint32_t i = 0; i < swapchain->get_max_frames_in_flight(); i++)
	{
		graphics_command_buffers.emplace_back(
			*device,
			device->get_graphics_command_pool(),
			true
		);
	}
}

static bool recreate_swapchain()
{
	// Wait for the device to idle.
	vkDeviceWaitIdle(*device);

	// Reset inflight fences,
	for (uint64_t i = 0; i < images_in_flight.size(); i++)
	{
		images_in_flight[i] = nullptr;
	}

	SwapchainInfo new_info = Swapchain::query_info(*device, surface);

	world_framebuffers.clear();
	delete ui_render_pass;
	delete world_render_pass;
	delete swapchain;

	try
	{
		world_render_pass = new RenderPass(
			*device,
			new_info.image_format.format,
			new_info.depth_format,
			vector2ui { 0, 0 },
			vector2ui { new_info.swapchain_extent.width, new_info.swapchain_extent.height },
			vector4f { 0.4f, 0.5f, 0.6f, 0.0f },
			1.0f,
			0,
			RenderPassClearFlag::COLOR_BUFFER_FLAG | RenderPassClearFlag::DEPTH_BUFFER_FLAG |
				RenderPassClearFlag::STENCIL_BUFFER_FLAG,
			false,
			true
		);

		ui_render_pass = new RenderPass(
			*device,
			new_info.image_format.format,
			new_info.depth_format,
			vector2ui { 0, 0 },
			vector2ui { new_info.swapchain_extent.width, new_info.swapchain_extent.height },
			vector4f { 0.0f, 0.0f, 0.0f, 0.0f },
			1.0f,
			0,
			RenderPassClearFlag::NONE_FLAG,
			true,
			false
		);

		swapchain = new Swapchain(
			*device,
			surface,
			new_info,
			*ui_render_pass
		);

		// World framebuffers.
		size_t swapchain_image_count = swapchain->get_images().size();

		world_framebuffers.reserve(swapchain_image_count);

		for (size_t i = 0; i < swapchain_image_count; i++)
		{
			VkImageView attachments[2] = {
				swapchain->get_image_views()[i],
				swapchain->get_depth_attachments()[i].get_image_view()
			};

			world_framebuffers.emplace_back(
				*device,
				*world_render_pass,
				vulkan_get_framebuffer_size(),
				2,
				attachments
			);
		}
	}
	catch (std::exception e)
	{
		LERROR("Failed to recreate the swapchain");

		return false;
	}

	return true;
}

}
