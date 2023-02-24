#include "renderer/fence.h"

#include "core/logger.h"

bool lise_fence_create(VkDevice device, bool create_signaled, lise_fence* out_fence)
{
	out_fence->is_signaled = create_signaled;

	VkFenceCreateInfo fence_ci = {};
	fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	if (create_signaled) fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateFence(device, &fence_ci, NULL, &out_fence->handle) != VK_SUCCESS)
	{
		LERROR("Could not create fence.");
		return false;
	}

	return true;
}

void lise_fence_destroy(VkDevice device, lise_fence* fence)
{
	if (fence->handle)
	{
		vkDestroyFence(device, fence->handle, NULL);
		fence->handle = NULL;
	}

	fence->is_signaled = false;
}

bool lise_fence_wait(VkDevice device, lise_fence* fence, uint64_t timeout_ns)
{
	if (!fence->is_signaled)
	{
		VkResult result = vkWaitForFences(device, 1, &fence->handle, VK_TRUE, timeout_ns);

		switch (result)
		{
		case VK_SUCCESS:
			fence->is_signaled = true;
			return true;
		case VK_TIMEOUT:
			LWARN("Fence has timed out while waiting.");
			break;
		case VK_ERROR_DEVICE_LOST:
			LERROR("An error has occurred while waiting for a fence: VK_ERROR_DEVICE_LOST");
			break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			LERROR("An error has occurred while waiting for a fence: VK_ERROR_OUT_OF_HOST_MEMORY");
			break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			LERROR("An error has occurred while waiting for a fence: VK_ERROR_OUT_OF_DEVICE_MEMORY");
			break;
		default:
			LERROR("An unknown error has occurred while waiting for a fence.");
			break;
		}
	}
	else return true; // Do not wait for fence if it's already signaled

	return false;
}

bool lise_fence_reset(VkDevice device, lise_fence* fence)
{
	if (fence->is_signaled)
	{
		if (vkResetFences(device, 1, &fence->handle) != VK_SUCCESS)
		{
			LERROR("An error has occurred while resetting a fence.");
			return false;
		}

		fence->is_signaled = false;
	}

	return true;
}
