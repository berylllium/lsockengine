#pragma once

#include <vulkan/vulkan.h>

#include "definitions.hpp"
#include "renderer/device.hpp"

namespace lise
{

enum class CommandBufferState
{
	READY,
	RECORDING,
	IN_RENDER_PASS,
	RECORDING_ENDED,
	SUBMITTED,
	NOT_ALLOCATED
};

class CommandBuffer
{
public:
	CommandBuffer(const Device& device, VkCommandPool command_pool, bool is_primary = true);

	CommandBuffer(CommandBuffer&& other);

	CommandBuffer(CommandBuffer&) = delete; // Prevent copies.

	~CommandBuffer();

	operator const VkCommandBuffer&() const;

	void reset();

	void begin(bool is_single_use, bool is_render_pass_continue, bool is_simultaneous_use);

	void end();

	void end_and_submit_single_use(VkQueue queue);

	void set_state(CommandBufferState state);
	CommandBufferState get_state() const;

private:
	VkCommandBuffer handle;

	/**
	 * @brief The current state of the command buffer.
	 */
	CommandBufferState state;

	/**
	 * @brief Cached device used when creating the command buffer.
	 */
	const Device& device;

	/**
	 * @brief Cached command pool used when creating the command buffer.
	 */
	VkCommandPool command_pool;

};

}
