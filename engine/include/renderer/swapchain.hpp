#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "renderer/device.hpp"
#include "renderer/vulkan_image.hpp"
#include "renderer/framebuffer.hpp"

#include "definitions.hpp"

namespace lise
{

struct SwapchainInfo
{
	VkSurfaceFormatKHR image_format;
	VkPresentModeKHR present_mode;
	VkExtent2D swapchain_extent;

	VkFormat depth_format;

	uint32_t min_image_count;
};

class Swapchain
{
public:
	Swapchain(
		const Device& device, 
		VkSurfaceKHR surface,
		SwapchainInfo swapchain_info,
		const RenderPass& render_pass
	);

	Swapchain(const Swapchain&) = delete; // Prevent copies.

	~Swapchain();

	bool acquire_next_image_index(
		uint64_t timeout_ns,
		VkSemaphore image_available_semaphore,
		VkFence fence,
		uint32_t& out_image_index
	);

	bool present(VkSemaphore render_complete_semaphore, uint32_t present_image_index);

	const SwapchainInfo& get_swapchain_info() const;

	const std::vector<Framebuffer>& get_framebuffers() const;

	const std::vector<VkImage>& get_images() const;
	
	const std::vector<VkImageView>& get_image_views() const;

	const std::vector<VulkanImage>& get_depth_attachments() const;

	uint8_t get_max_frames_in_flight() const;

	uint8_t get_current_frame() const;

	bool is_swapchain_out_of_date() const;

	static SwapchainInfo query_info(const Device& device, VkSurfaceKHR surface);

private:
	VkSwapchainKHR handle;

	SwapchainInfo swapchain_info;

	VkSurfaceFormatKHR image_format;

	std::vector<VkImage> images;
	std::vector<VkImageView> image_views;

	uint8_t max_frames_in_flight;
	uint8_t current_frame;
	
	VkFormat depth_format;
	std::vector<VulkanImage> depth_attachments;

	std::vector<Framebuffer> framebuffers;

	bool swapchain_out_of_date;

	const Device& device;
	const VkSurfaceKHR& surface;
};

}
