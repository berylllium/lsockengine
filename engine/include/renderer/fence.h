#pragma once

#include <vulkan/vulkan.h>

#include "definitions.h"

typedef struct lise_fence
{
	VkFence handle;
	bool is_signaled;
} lise_fence;

bool lise_fence_create(VkDevice device, bool create_signaled, lise_fence* out_fence);

void lise_fence_destroy(VkDevice device, lise_fence* fence);

bool lise_fence_wait(VkDevice device, lise_fence* fence, uint64_t timeout_ns);

bool lise_fence_reset(VkDevice device, lise_fence* fence);
