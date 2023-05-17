#pragma once

#include "definitions.hpp"
#include "renderer/device.hpp"
#include "renderer/resource/texture.hpp"

namespace lise
{

bool texture_system_initialize(const Device& device);

void texture_system_shutdown(const Device& device);

const Texture& texture_system_get_default_texture();

const Texture& texture_system_load(const Device& device, const std::string& path);

const Texture& texture_system_get(const std::string& path);

const Texture& texture_system_get_or_load(const Device& device, const std::string& path);

}
