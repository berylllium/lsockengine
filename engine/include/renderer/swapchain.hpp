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

	bool swapchain_present(VkSemaphore render_complete_semaphore, uint32_t present_image_index);

	static SwapchainInfo query_info(const Device& device, VkSurfaceKHR surface);

private:
	VkSwapchainKHR handle;

	SwapchainInfo swapchain_info;

	VkSurfaceFormatKHR image_format;
	uint32_t image_count;
	VkImage* images;
	VkImageView* image_views;

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
