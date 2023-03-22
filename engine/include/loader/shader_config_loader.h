#pragma once

#include "definitions.h"

typedef struct lise_shader_config_attribute
{
	char* type;
	char* name;
} lise_shader_config_attribute;

typedef struct lise_shader_config_uniform
{
	char* type;
	uint32_t scope;
	char* name;
} lise_shader_config_uniform;

typedef struct lise_shader_config
{
	char* name;

	uint32_t stage_count;
	char** stage_names;
	char** stage_file_names;

	char* render_pass_name;

	uint32_t attribute_count;
	lise_shader_config_attribute* attributes;

	uint32_t uniform_count;
	lise_shader_config_uniform* uniforms;
} lise_shader_config;

bool lise_shader_config_load(const char* path, lise_shader_config* out_config);

void lise_shader_config_free(lise_shader_config* config);
