#pragma once

#include "definitions.h"
#include "renderer/device.h"
#include "renderer/vulkan_image.h"

typedef struct lise_texture
{
	char* path;
	uint32_t id;
	uint32_t width;
	uint32_t height;
	uint8_t channel_count;

	lise_vulkan_image image;
	VkSampler sampler;
} lise_texture;

bool lise_texture_create_from_path(const lise_device* device, const char* path, lise_texture* out_texture);	

bool lise_texture_create(
	const lise_device* device,
	const char* path,
	uint32_t width,
	uint32_t height,
	uint32_t channel_count,
	const uint8_t* data,
	bool has_transparency,
	lise_texture* out_texture
);

void lise_texture_free(VkDevice device, lise_texture* texture);
