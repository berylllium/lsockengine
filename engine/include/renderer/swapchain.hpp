#pragma once

#include <vector>
#include <optional>

#include "renderer/device.hpp"
#include "renderer/vulkan_image.hpp"
#include "renderer/render_pass.hpp"

#include "definitions.hpp"

namespace lise
{

struct SwapchainInfo
{
	vk::SurfaceFormatKHR image_format;
	vk::PresentModeKHR present_mode;
	vk::Extent2D swapchain_extent;

	vk::Format depth_format;

	uint32_t min_image_count;
};

struct Swapchain
{
	vk::SwapchainKHR handle;

	SwapchainInfo swapchain_info;

	vk::SurfaceFormatKHR image_format;

	std::vector<vk::Image> images;
	std::vector<vk::ImageView> image_views;

	uint8_t max_frames_in_flight;
	uint8_t current_frame = 0;
	
	vk::Format depth_format;
	std::vector<std::unique_ptr<Image>> depth_attachments;

	std::vector<vk::Framebuffer> framebuffers;

	bool swapchain_out_of_date = false;

	vk::SurfaceKHR surface;

	const Device* device;

	Swapchain() = default;

	Swapchain(const Swapchain&) = delete; // Prevent copies.

	~Swapchain();

	static std::unique_ptr<Swapchain> create(
		const Device* device,
		const RenderPass* render_pass,
		vk::SurfaceKHR surface,
		SwapchainInfo swapchain_info
	);

	std::optional<uint32_t> acquire_next_image_index(
		uint64_t timeout_ns,
		vk::Semaphore image_available_semaphore,
		vk::Fence fence
	);

	bool present(vk::Semaphore render_complete_semaphore, uint32_t present_image_index);

	static SwapchainInfo query_info(const Device* device, vk::SurfaceKHR surface);
};

}
