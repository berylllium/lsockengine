#include "renderer/fence.hpp"

#include <simple-logger.hpp>

namespace lise
{

std::unique_ptr<Fence> Fence::create(const Device* device, bool create_signaled)
{
	auto out = std::make_unique<Fence>();

	// Copy trivial data.
	out->device = device;
	out->is_signaled = create_signaled;

	vk::FenceCreateInfo fence_ci;

	if (create_signaled) fence_ci.flags = vk::FenceCreateFlagBits::eSignaled;

	vk::Result r;

	std::tie(r, out->handle) = device->logical_device.createFence(fence_ci);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Could not create fence.");
		return nullptr;
	}

	return out;
}

Fence::~Fence()
{
	if (handle)
	{
		device->logical_device.destroy(handle);
	}
}

bool Fence::wait(uint64_t timeout_ns)
{
	if (!is_signaled)
	{
		vk::Result r = device->logical_device.waitForFences(1, &handle, vk::True, timeout_ns);

		switch (r)
		{
		case vk::Result::eSuccess:
			is_signaled = true;
			return true;
		case vk::Result::eTimeout:
			sl::log_warn("Fence has timed out while waiting.");
			break;
		case vk::Result::eErrorDeviceLost:
			sl::log_error("An error has occurred while waiting for a fence: VK_ERROR_DEVICE_LOST");
			break;
		case vk::Result::eErrorOutOfHostMemory:
			sl::log_error("An error has occurred while waiting for a fence: VK_ERROR_OUT_OF_HOST_MEMORY");
			break;
		case vk::Result::eErrorOutOfDeviceMemory:
			sl::log_error("An error has occurred while waiting for a fence: VK_ERROR_OUT_OF_DEVICE_MEMORY");
			break;
		default:
			sl::log_error("An unknown error has occurred while waiting for a fence.");
			break;
		}
	}
	else return true; // Do not wait for fence if it's already signaled

	return false;
}

bool Fence::reset()
{
	if (is_signaled)
	{
		vk::Result r = device->logical_device.resetFences(1, &handle);

		if (r != vk::Result::eSuccess)
		{
			sl::log_error("An error has occurred while resetting a fence.");
			return false;
		}

		is_signaled = false;
	}

	return true;
}

}
