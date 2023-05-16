#include "renderer/command_buffer.hpp"

namespace lise
{

CommandBuffer::CommandBuffer(const Device& device, VkCommandPool command_pool, bool is_primary)
	: device(device), command_pool(command_pool)
{
	VkCommandBufferAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocate_info.commandPool = command_pool;
	allocate_info.level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocate_info.commandBufferCount = 1;
	
	state = CommandBufferState::NOT_ALLOCATED;

	// TODO: Error handling.
	vkAllocateCommandBuffers(
		device,
		&allocate_info,
		&handle
	);

	state = CommandBufferState::READY;
}

CommandBuffer::CommandBuffer(CommandBuffer&& other) : device(other.device), command_pool(other.command_pool)
{
	handle = other.handle;
	other.handle = nullptr;

	state = other.state;
	other.state = CommandBufferState::NOT_ALLOCATED;
}

CommandBuffer::~CommandBuffer()
{
	if (handle)
	{
		vkFreeCommandBuffers(device, command_pool, 1, &handle);
	}
}

CommandBuffer::operator VkCommandBuffer() const
{
	return handle;
}

void CommandBuffer::begin(bool is_single_use, bool is_render_pass_continue, bool is_simultaneous_use)
{
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (is_single_use) {
		begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	}
	if (is_render_pass_continue) {
		begin_info.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	}
	if (is_simultaneous_use) {
		begin_info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	}

	vkBeginCommandBuffer(handle, &begin_info);

	state = CommandBufferState::RECORDING;
}

void CommandBuffer::end()
{
	vkEndCommandBuffer(handle);

	state = CommandBufferState::RECORDING_ENDED;
}

void CommandBuffer::end_and_submit_single_use(VkQueue queue)
{
	// End buffer
	end();

	// Submit queue
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &handle;

	vkQueueSubmit(queue, 1, &submit_info, NULL);

	// Wait for queue
	vkQueueWaitIdle(queue);
}

void CommandBuffer::set_state(CommandBufferState state)
{
	this->state = state;
}

CommandBufferState CommandBuffer::get_state() const
{
	return state;
}

}
