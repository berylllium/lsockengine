#include "renderer/command_buffer.h"

#include <string.h>

void lise_command_buffer_allocate(
	VkDevice device,
	VkCommandPool command_pool,
	bool is_primary,
	lise_command_buffer* out_command_buffer
)
{
	memset(out_command_buffer, 0, sizeof(lise_command_buffer));

	VkCommandBufferAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocate_info.commandPool = command_pool;
	allocate_info.level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocate_info.commandBufferCount = 1;
	
	out_command_buffer->state = LISE_COMMAND_BUFFER_STATE_NOT_ALLOCATED;

	vkAllocateCommandBuffers(
		device,
		&allocate_info,
		&out_command_buffer->handle
	);

	out_command_buffer->state = LISE_COMMAND_BUFFER_STATE_READY;
}

void lise_command_buffer_free(
	VkDevice device,
	VkCommandPool command_pool,
	lise_command_buffer* command_buffer
)
{
	vkFreeCommandBuffers(device, command_pool, 1, &command_buffer->handle);

	command_buffer->handle = 0;
	command_buffer->state = LISE_COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}

void lise_command_buffer_begin(
	lise_command_buffer* command_buffer,
	bool is_single_use,
	bool is_render_pass_continue,
	bool is_simultaneous_use
)
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

	vkBeginCommandBuffer(command_buffer->handle, &begin_info);

	command_buffer->state = LISE_COMMAND_BUFFER_STATE_RECORDING;
}

void lise_command_buffer_end(lise_command_buffer* command_buffer)
{
	vkEndCommandBuffer(command_buffer->handle);

	command_buffer->state = LISE_COMMAND_BUFFER_STATE_RECORDING_ENDED;
}

void lise_command_buffer_update_submitted(lise_command_buffer* command_buffer)
{
	command_buffer->state = LISE_COMMAND_BUFFER_STATE_SUBMITTED;
}

void lise_command_buffer_reset(lise_command_buffer* command_buffer)
{
	command_buffer->state = LISE_COMMAND_BUFFER_STATE_READY;
}

void lise_command_buffer_allocate_and_begin_single_use(
	VkDevice device,
	VkCommandPool command_pool,
	lise_command_buffer* out_command_buffer
)
{
	lise_command_buffer_allocate(device, command_pool, true, out_command_buffer);
	lise_command_buffer_begin(out_command_buffer, true, false, false);
}

void lise_command_buffer_end_and_submit_single_use(
	VkDevice device,
	VkCommandPool command_pool,
	lise_command_buffer* command_buffer,
	VkQueue queue
)
{
	// End buffer
	lise_command_buffer_end(command_buffer);

	// Submit queue
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer->handle;

	vkQueueSubmit(queue, 1, &submit_info, NULL);

	// Wait for queue
	vkQueueWaitIdle(queue);

	// Free command buffer
	lise_command_buffer_free(device, command_pool, command_buffer);
}
