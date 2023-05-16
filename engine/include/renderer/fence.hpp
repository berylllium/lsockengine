#pragma once

#include <vulkan/vulkan.h>

#include "renderer/device.hpp"
#include "definitions.hpp"

namespace lise
{

class Fence
{
public:
	Fence(const Device& device, bool create_signaled);

	Fence(Fence&& other);

	Fence(const Fence&) = delete; // Prevent copies.

	~Fence();

	Fence& operator = (const Fence&) = delete; // Prevent copies.

	bool wait(uint64_t timeout_ns);

	bool reset();

private:
	VkFence handle;
	bool is_signaled;

	const Device& device;
};

}
