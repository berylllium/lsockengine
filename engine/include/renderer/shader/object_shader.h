#pragma once

#include "definitions.h"
#include "renderer/shader_stage.h"
#include "renderer/pipeline.h"

#define LOBJECT_SHADER_STAGE_COUNT 2

typedef struct lise_object_shader
{
	lise_pipeline pipeline;
} lise_object_shader;

bool lise_object_shader_create(
	VkDevice device,
	lise_render_pass* render_pass,
	uint32_t framebuffer_width,
	uint32_t framebuffer_height,
	lise_object_shader* out_object_shader
);

void lise_object_shader_destroy(VkDevice device, lise_object_shader* object_shader);

void lise_object_shader_use(lise_command_buffer* command_buffer, lise_object_shader* object_shader);
