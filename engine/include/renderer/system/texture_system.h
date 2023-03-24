#pragma once

#include "definitions.h"
#include "renderer/device.h"
#include "renderer/resource/texture.h"

bool lise_texture_system_initialize(const lise_device* device);

void lise_texture_system_shutdown(VkDevice device);

const lise_texture* lise_texture_system_get_default_texture();

bool lise_texture_system_load(const lise_device* device, const char* path, lise_texture** out_texture);

bool lise_texture_system_get(const lise_device* device, const char* path, lise_texture** out_texture);

bool lise_texture_system_get_or_load(const lise_device* device, const char* path, lise_texture** out_texture);
