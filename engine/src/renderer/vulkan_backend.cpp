#include "renderer/vulkan_backend.hpp"

#include <memory>
#include <cstring>

#include <simple-logger.hpp>

#include "platform/platform.hpp"
#include "renderer/vulkan_platform.hpp"
#include "renderer/resource/model.hpp"

#include "renderer/system/texture_system.hpp"
#include "renderer/system/shader_system.hpp"

#include "node/node_tree.hpp"

namespace lise
{

// State variables.
static vk::Instance instance;

static vk::SurfaceKHR surface;

static Device* device;

static Swapchain* swapchain;
static RenderPass* world_render_pass;
static RenderPass* ui_render_pass;

static std::vector<std::unique_ptr<CommandBuffer>> graphics_command_buffers;

static std::vector<vk::Semaphore> image_available_semaphores;

static std::vector<vk::Semaphore> queue_complete_semaphores;

static std::vector<std::unique_ptr<Fence>> in_flight_fences;

static std::vector<Fence*> images_in_flight;

static uint32_t current_image_index;

static Shader* object_shader;

static Shader* ui_shader;

static std::vector<vk::Framebuffer> world_framebuffers;

#ifdef NDEBUG
	static constexpr bool enable_validation_layers = false;
#else
	static constexpr bool enable_validation_layers = true;
#endif

// Hardcoded validation layers
static std::vector<const char*> validation_layers = {
	"VK_LAYER_KHRONOS_validation"
};

// Hardcoded device extensions
static std::vector<const char*> device_extensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Static helper functions.
static bool check_validation_layer_support();
static bool create_command_buffers();
static bool recreate_swapchain();

// TODO: temp statics
static mat4x4 view_matrix = LMAT4X4_IDENTITY;
static Texture* temp_texture;

//static Model test_model;
static Model* car_model;
static Model* car2;

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
		sl::log_fatal("One or more requested validation layers do not exist.");
		return false;
	}

	// Vulkan Instance 
	vk::ApplicationInfo app_info(
		application_name,
		VK_MAKE_VERSION(0, 1, 0),
		"Lipin Sock Engine",
		VK_MAKE_VERSION(0, 1, 0),
		VK_API_VERSION_1_3
	);

	// Instance Extensions
	auto platform_extensions = platform_get_required_instance_extensions();

	// Instance validation layers.
	auto enabled_validation_layers = validation_layers;

	if (!enable_validation_layers)
	{
		enabled_validation_layers.clear();
	}

	// Instance creation.
	vk::InstanceCreateInfo create_info(
		{},
		&app_info,
		enabled_validation_layers,
		platform_extensions
	);

	vk::Result r;

	std::tie(r, instance) = vk::createInstance(create_info);

	// Create vulkan instance
	if (r != vk::Result::eSuccess)
	{
		sl::log_fatal("Failed to create Vulkan instance.");
		return false;
	}

	// Create vulkan surface
	auto _surface = vulkan_platform_create_vulkan_surface(instance);

	if (!_surface)
	{
		sl::log_fatal("Failed to create vulkan surface.");
		return false;
	}

	surface = *_surface;

	// Create the device
	std::vector<std::string> device_extensions_str(device_extensions.size());
	for (size_t i = 0; i < device_extensions.size(); i++)
	{
		device_extensions_str[i] = device_extensions[i];
	}

	std::vector<std::string> validation_layers_str(validation_layers.size());
	for (size_t i = 0; i < validation_layers.size(); i++)
	{
		validation_layers_str[i] = validation_layers[i];
	}

	device = Device::create(
		instance,
		device_extensions_str,
		validation_layers_str,
		surface
	).release();

	if (device == nullptr)
	{
		sl::log_fatal("Failed to create a logical device.");
		return false;
	}

	// Get swapchain info
	SwapchainInfo swapchain_info = Swapchain::query_info(device, surface);

	// Create the render passs
	world_render_pass = RenderPass::create(
		device,
		swapchain_info.image_format.format,
		swapchain_info.depth_format,
		vector2ui { 0, 0 },
		vector2ui { swapchain_info.swapchain_extent.width, swapchain_info.swapchain_extent.height },
		vector4f { 0.4f, 0.5f, 0.6f, 0.0f },
		1.0f,
		0,
		RenderPassClearFlagBits::COLOR_BUFFER_FLAG | RenderPassClearFlagBits::DEPTH_BUFFER_FLAG |
			RenderPassClearFlagBits::STENCIL_BUFFER_FLAG,
		false,
		true
	).release();

	ui_render_pass = RenderPass::create(
		device,
		swapchain_info.image_format.format,
		swapchain_info.depth_format,
		vector2ui { 0, 0 },
		vector2ui { swapchain_info.swapchain_extent.width, swapchain_info.swapchain_extent.height },
		vector4f { 0.0f, 0.0f, 0.0f, 0.0f },
		1.0f,
		0,
		RenderPassClearFlagBits::NONE_FLAG,
		true,
		false
	).release();
	
	if (!world_render_pass)
	{
		sl::log_fatal("Failed to create the world render pass.");
		return false;
	}

	if (!ui_render_pass)
	{
		sl::log_fatal("Failed to create the ui render pass.");
		return false;
	}

	// Create the swapchain
	swapchain = Swapchain::create(
		device,
		ui_render_pass,
		surface,
		swapchain_info
	).release();

	if (!swapchain)
	{
		sl::log_fatal("Failed to create the swapchain.");
		return false;
	}

	// Create world framebuffers.
	world_framebuffers.resize(swapchain->images.size());

	for (size_t i = 0; i < swapchain->images.size(); i++)
	{
		std::vector<vk::ImageView> attachments = {
			swapchain->image_views[i],
			swapchain->depth_attachments[i]->image_view
		};

		vk::FramebufferCreateInfo fb_ci(
			{},
			world_render_pass->handle,
			attachments,
			vulkan_get_framebuffer_size().w,
			vulkan_get_framebuffer_size().h,
			1
		);

		std::tie(r, world_framebuffers[i]) = device->logical_device.createFramebuffer(fb_ci);
	}

	// Create command buffers.
	create_command_buffers();

	// Create sync objects
	image_available_semaphores.resize(swapchain->max_frames_in_flight);

	queue_complete_semaphores.resize(swapchain->max_frames_in_flight);

	in_flight_fences.reserve(swapchain->max_frames_in_flight);

	for (uint32_t i = 0; i < swapchain->max_frames_in_flight; i++)
	{
		vk::SemaphoreCreateInfo semaphore_ci;

		std::tie(r, image_available_semaphores[i]) = device->logical_device.createSemaphore(semaphore_ci);

		if (r != vk::Result::eSuccess)
		{
			sl::log_error("Failed to create image available semaphore.");
			return false;
		}

		std::tie(r, queue_complete_semaphores[i]) = device->logical_device.createSemaphore(semaphore_ci);

		if (r != vk::Result::eSuccess)
		{
			sl::log_error("Failed to create queue complete semaphore.");
			return false;
		}

		auto fence = Fence::create(device, true);

		if (!fence)
		{
			sl::log_error("Failed to create fence.");
			return false;
		}

		in_flight_fences.push_back(std::move(fence)); 
	}

	// Preallocate the in flight images and set them to nullptr;
	images_in_flight.resize(swapchain->images.size(), nullptr);

	if (!texture_system_initialize(device))
	{
		sl::log_fatal("Failed to initialize the vulkan renderer texture subsystem.");
		return false;
	}

	if (!shader_system_initialize(device, swapchain))
	{
		sl::log_fatal("Failed to initialize the vulkan renderer shader subsystem.");
		return false;
	}

	// Load default shaders.
	object_shader = shader_system_load("assets/shaders/builtin.object_shader.scfg", world_render_pass);
	ui_shader = shader_system_load("assets/shaders/builtin.ui_shader.scfg", ui_render_pass);

	if (object_shader == nullptr)
	{
		sl::log_fatal("Failed to load the object shader.");
		
		return false;
	}

	if (ui_shader == nullptr)
	{
		sl::log_fatal("Failed to load the ui shader.");
		
		return false;
	}

	// TEMP:
	// Car model.
	std::optional<Obj> car_obj = Obj::load("assets/models/obj/car.obj");

	if (!car_obj)
	{
		sl::log_fatal("Failed to load car obj file.");

		return false;
	}

	car_model = Model::create(device, object_shader, *car_obj).release();
	car_model->transform.set_position(0, 0, -10);

	return true;
}

void vulkan_shutdown()
{
	vk::Result r = device->logical_device.waitIdle();

	delete car_model;

	shader_system_shutdown();

	texture_system_shutdown();

	for (size_t i = 0; i < image_available_semaphores.size(); i++)
	{
		device->logical_device.destroy(image_available_semaphores[i]);
		device->logical_device.destroy(queue_complete_semaphores[i]);
	}

	in_flight_fences.clear();

	graphics_command_buffers.clear();

	for (size_t i = 0; i < world_framebuffers.size(); i++)
	{
		device->logical_device.destroy(world_framebuffers[i]);
	}

	delete ui_render_pass;

	delete world_render_pass;

	delete swapchain;

	delete device;

	instance.destroy(surface);

	instance.destroy();

	sl::log_info("Successfully shut down the vulkan backend.");
}

bool vulkan_begin_frame(float delta_time)
{
	if (swapchain->swapchain_out_of_date)
	{
		recreate_swapchain();
		return vulkan_begin_frame(delta_time);
	}

	uint8_t current_frame = swapchain->current_frame;

	// Wait for the current frame
	if (!in_flight_fences[current_frame]->wait())
	{
		sl::log_warn("Failed to wait on an in-flight fence.");
		return false;
	}

	// Acquire next image in swapchain
	auto next_image_index = swapchain->acquire_next_image_index(
		UINT64_MAX, 
		image_available_semaphores[current_frame],
		nullptr
	);

	if (!next_image_index)
	{
		sl::log_warn("Failed to acquire next swapchain image");
		return false;
	}

	current_image_index = *next_image_index;

	// Begin command buffer
	CommandBuffer* command_buffer = graphics_command_buffers[current_frame].get();
	command_buffer->reset();
	command_buffer->begin(false, false, false);

	// Dynamic state
	vk::Viewport viewport;
	viewport.x = 0.0f;
	viewport.y = (float) swapchain->swapchain_info.swapchain_extent.height;
	viewport.width = (float) swapchain->swapchain_info.swapchain_extent.width;
	viewport.height = -(float) swapchain->swapchain_info.swapchain_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Scissor
	vk::Rect2D scissor;
	scissor.offset.x = scissor.offset.y = 0;
	scissor.extent.width = swapchain->swapchain_info.swapchain_extent.width;
	scissor.extent.height = swapchain->swapchain_info.swapchain_extent.height;

	command_buffer->handle.setViewport(0, 1, &viewport);
	command_buffer->handle.setScissor(0, 1, &scissor);

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
	car_model->transform.set_rotation(car_model->transform.get_rotation() + vector3f {0, LQUARTER_PI * delta_time});
	
	car_model->draw(command_buffer, current_frame);

	//lise_model_draw(&test_model, vulkan_context.device.logical_device, command_buffer->handle, vulkan_context.current_image_index);
	//
	//lise_model_draw(&car_model, vulkan_context.device.logical_device, command_buffer->handle, vulkan_context.current_image_index);

	// -------- ENDTEMP

	return true;
}

bool vulkan_end_frame(float delta_time)
{
	uint8_t current_frame = swapchain->current_frame;
	CommandBuffer* command_buffer = graphics_command_buffers[current_frame].get();

	// End render pass.
	world_render_pass->end(command_buffer);

	ui_render_pass->begin(command_buffer, swapchain->framebuffers[current_image_index]);

	ui_render_pass->end(command_buffer);

	command_buffer->end();

	// Wait if a previous frame is still using this image.
	if (images_in_flight[current_frame] != NULL)
	{
		images_in_flight[current_frame]->wait();
	}

	// Mark fence as being in use by this image.
	images_in_flight[current_frame] = in_flight_fences[current_frame].get();

	// Reset the fence
	images_in_flight[current_frame]->reset();

	// Submit queue
	vk::PipelineStageFlags stage_flags[1] = {
		vk::PipelineStageFlagBits::eColorAttachmentOutput
	};

	vk::SubmitInfo submit_info(
		1,
		&image_available_semaphores[current_frame],
		stage_flags,
		1,
		&command_buffer->handle,
		1,
		&queue_complete_semaphores[current_frame]
	);

	vk::Result r = device->graphics_queue.submit(1, &submit_info, in_flight_fences[current_frame]->handle);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to submit queue.");
		return false;
	}

	command_buffer->set_state(CommandBufferState::SUBMITTED);

	// Give images back to the swapchain
	if (!swapchain->present(queue_complete_semaphores[swapchain->current_frame], current_image_index) &&
		!swapchain->swapchain_out_of_date
	)
	{
		// Swapchain is not out of date, so the return value indicates a failure.
		sl::log_fatal("Failed to present swap chain image.");
		return false;
	}

	return true;
}

vector2ui vulkan_get_framebuffer_size()
{
	return vector2ui {
		swapchain->swapchain_info.swapchain_extent.width,
		swapchain->swapchain_info.swapchain_extent.height
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
	auto [r, available_layers] = vk::enumerateInstanceLayerProperties();

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to enumerate instane layer properties.");
		return false;
	}

	for (uint32_t i = 0; i < validation_layers.size(); i++)
	{
		bool layer_found = false;

		// Loop through available layers and check if our layers are among them
		for (uint32_t j = 0; j < available_layers.size(); j++)
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

bool create_command_buffers()
{
	sl::log_debug("Creating initial command buffers");

	// Clear old command buffers.
	graphics_command_buffers.clear();

	graphics_command_buffers.reserve(swapchain->max_frames_in_flight);

	for (uint32_t i = 0; i < swapchain->max_frames_in_flight; i++)
	{
		auto cb = CommandBuffer::create(device, device->graphics_command_pool, true);

		if (!cb)
		{
			sl::log_fatal("Failed to create command buffers");
			return false;
		}

		graphics_command_buffers.push_back(std::move(cb));
	}

	return true;
}

static bool recreate_swapchain()
{
	sl::log_debug("Recreating swapchain.");

	// Wait for the device to idle.
	vk::Result r = device->logical_device.waitIdle();

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to wait for device to idle.");
		return false;
	}

	// Reset inflight fences,
	for (uint64_t i = 0; i < images_in_flight.size(); i++)
	{
		images_in_flight[i] = nullptr;
	}

	SwapchainInfo new_info = Swapchain::query_info(device, surface);

	for (size_t i = 0; i < world_framebuffers.size(); i++)
	{
		device->logical_device.destroy(world_framebuffers[i]);
	}
	world_framebuffers.clear();

	delete ui_render_pass;
	delete world_render_pass;
	delete swapchain;

	world_render_pass = RenderPass::create(
		device,
		new_info.image_format.format,
		new_info.depth_format,
		vector2ui { 0, 0 },
		vector2ui { new_info.swapchain_extent.width, new_info.swapchain_extent.height },
		vector4f { 0.4f, 0.5f, 0.6f, 0.0f },
		1.0f,
		0,
		RenderPassClearFlagBits::COLOR_BUFFER_FLAG | RenderPassClearFlagBits::DEPTH_BUFFER_FLAG |
			RenderPassClearFlagBits::STENCIL_BUFFER_FLAG,
		false,
		true
	).release();

	ui_render_pass = RenderPass::create(
		device,
		new_info.image_format.format,
		new_info.depth_format,
		vector2ui { 0, 0 },
		vector2ui { new_info.swapchain_extent.width, new_info.swapchain_extent.height },
		vector4f { 0.0f, 0.0f, 0.0f, 0.0f },
		1.0f,
		0,
		RenderPassClearFlagBits::NONE_FLAG,
		true,
		false
	).release();

	if (!world_render_pass)
	{
		sl::log_fatal("Failed to recreate the world render pass.");
		return false;
	}

	if (!ui_render_pass)
	{
		sl::log_fatal("Failed to recreate the ui render pass.");
		return false;
	}

	swapchain = Swapchain::create(
		device,
		ui_render_pass,
		surface,
		new_info
	).release();

	if (!swapchain)
	{
		sl::log_fatal("Failed to recreate the swapchain.");
		return false;
	}

	// World framebuffers.
	size_t swapchain_image_count = swapchain->images.size();

	world_framebuffers.resize(swapchain_image_count);

	for (size_t i = 0; i < swapchain_image_count; i++)
	{
		std::vector<vk::ImageView> attachments = {
			swapchain->image_views[i],
			swapchain->depth_attachments[i]->image_view
		};

		vk::FramebufferCreateInfo fb_ci(
			{},
			world_render_pass->handle,
			attachments,
			vulkan_get_framebuffer_size().w,
			vulkan_get_framebuffer_size().h,
			1
		);

		std::tie(r, world_framebuffers[i]) = device->logical_device.createFramebuffer(fb_ci);

		if (r != vk::Result::eSuccess)
		{
			sl::log_error("Failed to recreate world frame buffers.");
			return false;
		}
	}

	return true;
}

}
