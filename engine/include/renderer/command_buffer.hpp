#pragma once

#include <memory>

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

struct CommandBuffer
{
	vk::CommandBuffer handle;

	/**
	 * @brief The current state of the command buffer.
	 */
	CommandBufferState state;

	/**
	 * @brief Cached device used when creating the command buffer.
	 */
	const Device* device;

	/**
	 * @brief Cached command pool used when creating the command buffer.
	 */
	vk::CommandPool command_pool;

	CommandBuffer() = default;

	CommandBuffer(CommandBuffer&) = delete; // Prevent copies.

	~CommandBuffer();

	static std::unique_ptr<CommandBuffer> create(
		const Device* device,
		vk::CommandPool command_pool,
		bool is_primary = true
	);

	void reset();

	bool begin(bool is_single_use, bool is_render_pass_continue, bool is_simultaneous_use);

	bool end();

	bool end_and_submit_single_use(vk::Queue queue);

	void set_state(CommandBufferState state);
};

}
