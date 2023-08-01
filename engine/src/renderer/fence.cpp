#include "renderer/fence.hpp"

#include <simple-logger.hpp>

namespace lise
{

Fence::Fence(const Device& device, bool create_signaled) : device(device), is_signaled(create_signaled)
{
	auto fence_ci = VkFenceCreateInfo {};
	fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	if (create_signaled) fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateFence(device, &fence_ci, NULL, &handle) != VK_SUCCESS)
	{
		sl::log_error("Could not create fence.");
		throw std::exception();
	}
}

Fence::Fence(Fence&& other) : device(other.device)
{
	handle = other.handle;
	other.handle = nullptr;

	is_signaled = other.is_signaled;
	other.is_signaled = false;
}

Fence::~Fence()
{
	if (handle)
	{
		vkDestroyFence(device, handle, NULL);
	}
}

Fence::operator VkFence() const
{
	return handle;
}

bool Fence::wait(uint64_t timeout_ns)
{
	if (!is_signaled)
	{
		VkResult result = vkWaitForFences(device, 1, &handle, VK_TRUE, timeout_ns);

		switch (result)
		{
		case VK_SUCCESS:
			is_signaled = true;
			return true;
		case VK_TIMEOUT:
			sl::log_warn("Fence has timed out while waiting.");
			break;
		case VK_ERROR_DEVICE_LOST:
			sl::log_error("An error has occurred while waiting for a fence: VK_ERROR_DEVICE_LOST");
			break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			sl::log_error("An error has occurred while waiting for a fence: VK_ERROR_OUT_OF_HOST_MEMORY");
			break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
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
		if (vkResetFences(device, 1, &handle) != VK_SUCCESS)
		{
			sl::log_error("An error has occurred while resetting a fence.");
			return false;
		}

		is_signaled = false;
	}

	return true;
}

}
