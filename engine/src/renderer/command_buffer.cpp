#include "renderer/command_buffer.hpp"

#include <simple-logger.hpp>

namespace lise
{

std::unique_ptr<CommandBuffer> CommandBuffer::create(
	const Device* device,
	vk::CommandPool command_pool,
	bool is_primary
)
{
	auto out = std::make_unique<CommandBuffer>();

	// Copy trivial data.
	out->device = device;
	out->command_pool = command_pool;
	
	vk::CommandBufferAllocateInfo allocate_info(
		command_pool,
		is_primary ? vk::CommandBufferLevel::ePrimary : vk::CommandBufferLevel::eSecondary
	);

	allocate_info.commandBufferCount = 1;
	
	out->state = CommandBufferState::NOT_ALLOCATED;

	auto [r, cbs] = out->device->logical_device.allocateCommandBuffers(allocate_info);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to allocate command buffers.");

		return nullptr;
	}

	out->handle = cbs[0];
	
	out->state = CommandBufferState::READY;

	return out;
}

CommandBuffer::~CommandBuffer()
{
	if (handle)
	{
		device->logical_device.freeCommandBuffers(command_pool, 1, &handle);
	}
}

void CommandBuffer::reset()
{
	state = CommandBufferState::READY;
}

bool CommandBuffer::begin(bool is_single_use, bool is_render_pass_continue, bool is_simultaneous_use)
{
	vk::CommandBufferBeginInfo begin_info = {};
	begin_info.sType = vk::StructureType::eCommandBufferBeginInfo;

	if (is_single_use) {
		begin_info.flags |= vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	}
	if (is_render_pass_continue) {
		begin_info.flags |= vk::CommandBufferUsageFlagBits::eRenderPassContinue;
	}
	if (is_simultaneous_use) {
		begin_info.flags |= vk::CommandBufferUsageFlagBits::eSimultaneousUse;
	}

	vk::Result r = handle.begin(begin_info);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to begin command buffer.");

		return false;
	}

	state = CommandBufferState::RECORDING;

	return true;
}

bool CommandBuffer::end()
{
	vk::Result r = handle.end();

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to end command buffer.");

		return false;
	}

	state = CommandBufferState::RECORDING_ENDED;

	return true;
}

bool CommandBuffer::end_and_submit_single_use(vk::Queue queue)
{
	// End buffer
	end();

	// Submit queue
	vk::SubmitInfo submit_info(
		0,
		nullptr,
		nullptr,
		1,
		&handle
	);

	vk::Result r = queue.submit(submit_info);

	if (r != vk::Result::eSuccess)
	{
		sl::log_error("Failed to submit command buffer to queue.");
		return false;
	}

	// Wait for queue
	r = queue.waitIdle();

	if (r != vk::Result::eSuccess)
	{
		sl::log_warn("Failed to wait for queue to idle.");
	}

	return true;
}

void CommandBuffer::set_state(CommandBufferState state)
{
	this->state = state;
}

}
