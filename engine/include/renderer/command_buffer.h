#pragma once

#include <vulkan/vulkan.h>

#include "definitions.h"

typedef enum lise_command_buffer_state {
    LISE_COMMAND_BUFFER_STATE_READY,
    LISE_COMMAND_BUFFER_STATE_RECORDING,
    LISE_COMMAND_BUFFER_STATE_IN_RENDER_PASS,
    LISE_COMMAND_BUFFER_STATE_RECORDING_ENDED,
    LISE_COMMAND_BUFFER_STATE_SUBMITTED,
    LISE_COMMAND_BUFFER_STATE_NOT_ALLOCATED
} lise_command_buffer_state;

typedef struct lise_command_buffer {
    VkCommandBuffer handle;

    // Command buffer state.
    lise_command_buffer_state state;
} lise_command_buffer;

void lise_command_buffer_allocate(
    VkDevice device,
    VkCommandPool command_pool,
    bool is_primary,
    lise_command_buffer* out_command_buffer
);

void lise_command_buffer_free(
    VkDevice device,
    VkCommandPool command_pool,
    lise_command_buffer* command_buffer
);

void lise_command_buffer_begin(
    lise_command_buffer* command_buffer,
    bool is_single_use,
    bool is_render_pass_continue,
    bool is_simultaneous_use
);

void lise_command_buffer_end(lise_command_buffer* command_buffer);

void lise_command_buffer_update_submitted(lise_command_buffer* command_buffer);

void lise_command_buffer_reset(lise_command_buffer* command_buffer);

void lise_command_buffer_allocate_and_begin_single_use(
    VkDevice device,
    VkCommandPool command_pool,
    lise_command_buffer* out_command_buffer
);

void lise_command_buffer_end_and_submit_single_use(
    VkDevice device,
    VkCommandPool command_pool,
    lise_command_buffer* command_buffer,
    VkQueue queue
);
