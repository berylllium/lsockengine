#include "renderer/swapchain.hpp"

#include <simple-logger.hpp>

namespace lise
{

std::unique_ptr<Swapchain> Swapchain::create(
	const Device* device,
	const RenderPass* render_pass,
	vk::SurfaceKHR surface,
	SwapchainInfo swapchain_info
)
{
	auto out = std::make_unique<Swapchain>();

	// Copy trivial data.
	out->device = device;
	out->surface = surface;
	out->swapchain_info = swapchain_info;

	vk::SwapchainCreateInfoKHR swap_chain_ci(
		{},
		out->surface,
		out->swapchain_info.min_image_count,
		out->swapchain_info.image_format.format,
		out->swapchain_info.image_format.colorSpace,
		out->swapchain_info.swapchain_extent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment
	);

	auto device_queue_indices = device->queue_indices;

	uint32_t queue_indices[] = { device_queue_indices.graphics_queue_index, device_queue_indices.present_queue_index };

	if (device_queue_indices.graphics_queue_index != device_queue_indices.present_queue_index)
	{
		swap_chain_ci.imageSharingMode = vk::SharingMode::eConcurrent;
		swap_chain_ci.queueFamilyIndexCount = 2;
		swap_chain_ci.pQueueFamilyIndices = queue_indices;
	}
	else
	{
		swap_chain_ci.imageSharingMode = vk::SharingMode::eExclusive;
	}

	auto device_swap_chain_support_info = Device::query_swapchain_support(device->physical_device, surface);

	swap_chain_ci.preTransform = device_swap_chain_support_info.surface_capabilities.currentTransform;
	swap_chain_ci.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swap_chain_ci.presentMode = swapchain_info.present_mode;
	swap_chain_ci.clipped = vk::True;
	swap_chain_ci.oldSwapchain = nullptr;

	vk::Result r;

	std::tie(r, out->handle) = device->logical_device.createSwapchainKHR(swap_chain_ci);

	if (r != vk::Result::eSuccess)
	{
		sl::log_fatal("Failed to create the swap chain.");
		return nullptr;
	}

	// Get swapchain images
	std::tie(r, out->images) = device->logical_device.getSwapchainImagesKHR(out->handle);

	out->max_frames_in_flight = out->images.size() - 1;
	
	// Views
	out->image_views.resize(out->images.size());

	for (size_t i = 0; i < out->images.size(); i++)
	{
		vk::ImageViewCreateInfo view_ci(
			{},
			out->images[i],
			vk::ImageViewType::e2D,
			swapchain_info.image_format.format,
			{},
			vk::ImageSubresourceRange(
				vk::ImageAspectFlagBits::eColor,
				0,
				1,
				0,
				1
			)
		);

		std::tie(r, out->image_views[i]) = device->logical_device.createImageView(view_ci);

		if (r != vk::Result::eSuccess)
		{
			sl::log_error("Failed to create swapchain image view.");
			return nullptr;
		}
	}

	// Create the depth attachments
	out->depth_attachments.reserve(out->images.size());

	for (size_t i = 0; i < out->images.size(); i++)
	{
		auto depth_attachment = Image::create(
			device,
			vk::ImageType::e2D,
			vector2ui { swapchain_info.swapchain_extent.width, swapchain_info.swapchain_extent.height },
			swapchain_info.depth_format,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eDepthStencilAttachment,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			true,
			vk::ImageAspectFlagBits::eDepth
		);

		if(!depth_attachment)
		{
			sl::log_fatal("Failed to create image swap chain depth attachments.");
			return nullptr;
		}

		out->depth_attachments.push_back(std::move(depth_attachment));
	}

	// Create framebuffers
	out->framebuffers.resize(out->images.size());

	for (uint32_t i = 0; i < out->images.size(); i++)
	{
		std::vector<vk::ImageView> attachments = { out->image_views[i] };

		vk::FramebufferCreateInfo fb_ci(
			{},
			render_pass->handle,
			attachments,
			swapchain_info.swapchain_extent.width,
			swapchain_info.swapchain_extent.height,
			1
		);

		std::tie(r, out->framebuffers[i]) = device->logical_device.createFramebuffer(fb_ci);

		if (r != vk::Result::eSuccess)
		{
			sl::log_fatal("Failed to create swap chain frame buffers");
			return nullptr;
		}
	}

	return out;
}

Swapchain::~Swapchain()
{
	for (size_t i = 0; i < framebuffers.size(); i++)
	{
		device->logical_device.destroy(framebuffers[i]);
	}

	for (uint32_t i = 0; i < images.size(); i++)
	{
		device->logical_device.destroy(image_views[i]);
	}

	device->logical_device.destroy(handle);
}

std::optional<uint32_t> Swapchain::acquire_next_image_index(
	uint64_t timeout_ns,
	vk::Semaphore image_available_semaphore,
	vk::Fence fence
)
{
	uint32_t out_image_index;

	vk::Result r;

	std::tie(r, out_image_index) = device->logical_device.acquireNextImageKHR(handle, timeout_ns, image_available_semaphore, fence);

	if (r == vk::Result::eErrorOutOfDateKHR)
	{
		sl::log_debug("Swapchain is out of date. Attempring to recreate.");
		swapchain_out_of_date = true;
		return {};
	}
	else if (r != vk::Result::eSuccess && r != vk::Result::eSuboptimalKHR)
	{
		sl::log_fatal("Failed to acquire next swapchain image.");
		return {};
	}

	return out_image_index;
}

bool Swapchain::present(vk::Semaphore render_complete_semaphore, uint32_t present_image_index)
{
	vk::PresentInfoKHR present_info(
		1, &render_complete_semaphore,
		1, &handle, &present_image_index
	);

	vk::Result r = device->present_queue.presentKHR(present_info);

	if (r == vk::Result::eErrorOutOfDateKHR || r == vk::Result::eSuboptimalKHR)
	{
		sl::log_debug("Swapchain is out of date. Attempring to recreate.");
		swapchain_out_of_date = true;
		return false;
	}
	else if (r != vk::Result::eSuccess)
	{
		sl::log_fatal("Failed to present swapchain image.");
		return false;
	}

	current_frame = (current_frame + 1) % max_frames_in_flight;

	return true;
}

SwapchainInfo Swapchain::query_info(const Device* device, vk::SurfaceKHR surface)
{
	SwapchainInfo info = {};

	auto swap_chain_support_info = Device::query_swapchain_support(device->physical_device, surface);
	
	// Choose swap surface format
	info.image_format = swap_chain_support_info.surface_formats[0]; // Default, first surface format.

	for (auto& surface_format : swap_chain_support_info.surface_formats)
	{
		if (surface_format.format == vk::Format::eB8G8R8A8Srgb &&
			surface_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			info.image_format = surface_format;
			break;
		}
	}

	// Choose swap present mode
	info.present_mode = vk::PresentModeKHR::eFifo; // Default, guaranteed present mode

	for (auto& present_mode : swap_chain_support_info.present_modes)
	{
		if (present_mode == vk::PresentModeKHR::eMailbox)
		{
			info.present_mode = present_mode;
			break;
		}
	}

	// Choose swap extent
	info.swapchain_extent = swap_chain_support_info.surface_capabilities.currentExtent;

	auto swap_chain_image_count = swap_chain_support_info.surface_capabilities.minImageCount + 1;

	// Make sure to not exceed the maximum image count
	if (swap_chain_image_count > swap_chain_support_info.surface_capabilities.maxImageCount &&
		swap_chain_support_info.surface_capabilities.maxImageCount > 0)
	{
		swap_chain_image_count = swap_chain_support_info.surface_capabilities.maxImageCount;
	}

	info.min_image_count = swap_chain_image_count;

	// Check if device supports depth format
	const uint32_t candidate_count = 3;
	const vk::Format candidates[3] = {
		vk::Format::eD32Sfloat,
		vk::Format::eD32SfloatS8Uint,
		vk::Format::eD24UnormS8Uint
	};

	float formats_supported = false;
	vk::Format depth_format;
	auto flags = vk::FormatFeatureFlagBits::eDepthStencilAttachment;
	for (uint32_t i = 0; i < candidate_count; i++)
	{
		vk::FormatProperties format_properties = device->physical_device.getFormatProperties(candidates[i]);

		if ((format_properties.linearTilingFeatures & flags) == flags)
		{
			depth_format = candidates[i];
			formats_supported = true;
			break;
		}
		else if ((format_properties.optimalTilingFeatures & flags) == flags)
		{
			depth_format = candidates[i];
			formats_supported = true;
			break;
		}
	}

	if (!formats_supported)
	{
		sl::log_error("Failed to find supported depth format during swapchain creation.");
		return SwapchainInfo {};
	}

	info.depth_format = depth_format;

	return info;
}

}
