#pragma once

#include <vulkan/vulkan.h>

#include "renderer/device.hpp"
#include "definitions.hpp"

namespace lise
{

struct Fence
{
	vk::Fence handle;
	bool is_signaled;

	const Device* device;

	Fence() = default;

	Fence(const Fence&) = delete; // Prevent copies.

	~Fence();
	
	LAPI static std::unique_ptr<Fence> create(const Device* device, bool create_signaled);

	Fence& operator = (const Fence&) = delete; // Prevent copies.

	bool wait(uint64_t timeout_ns = UINT64_MAX);

	bool reset();
};

}
