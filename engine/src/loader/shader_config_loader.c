#include "loader/shader_config_loader.h"

#include <stdlib.h>
#include <string.h>

#include <container/darray.h>

#include "core/logger.h"
#include "loader/obj_format_loader.h"

bool lise_shader_config_load(const char* path, lise_shader_config* out_config)
{
	lise_obj_format loaded_format;

	if (!lise_obj_format_load(path, &loaded_format))
	{
		LERROR("Failed to load shader config for shader: `%s`.", path);
		return false;
	}

	uint64_t found_line_count;
	lise_obj_format_line** found_lines;

	// Get the name.
	lise_obj_format_get_line(&loaded_format, "name", &found_line_count, &found_lines);

	if (found_line_count != 1)
	{
		LERROR("Provided config file `%s` %s",
			path,
			found_line_count == 0 ?
				"does not contain a `name` line." :
				"contains too many `name` lines. Please provide only one."
		);

		lise_obj_format_free(&loaded_format);
		return false;
	}

	if (found_lines[0]->token_count != 1)
	{
		LERROR("Provided config file `%s` contains a `name` line, %s",
			path,
			found_lines[0]->token_count == 0 ?
				"but no names were specified. Please specify one." :
				"but too many names were specified. Please specify only one."
		);

		lise_obj_format_free(&loaded_format);
		return false;
	}

	out_config->name = strdup(found_lines[0]->tokens[0]);

	// Get the render pass name.
	lise_obj_format_get_line(&loaded_format, "render_pass", &found_line_count, &found_lines);

	if (found_line_count != 1)
	{
		LERROR("Provided config file `%s` %s",
			path,
			found_line_count == 0 ?
				"does not contain a `render_pass` line." :
				"contains too many `render_pass` lines. Please provide only one."
		);

		lise_obj_format_free(&loaded_format);
		return false;
	}

	if (found_lines[0]->token_count != 1)
	{
		LERROR("Provided config file `%s` contains a `render_pass` line, %s",
			path,
			found_lines[0]->token_count == 0 ?
				"but no render pass names were specified. Please specify one." :
				"but too many render pass names were specified. Please specify only one."
		);

		lise_obj_format_free(&loaded_format);
		return false;
	}

	out_config->render_pass_name = strdup(found_lines[0]->tokens[0]);

	// Load stage names.
	lise_obj_format_get_line(&loaded_format, "stages", &found_line_count, &found_lines);

	if (found_line_count != 1)
	{
		LERROR("Provided config file `%s` %s",
			path,
			found_line_count == 0 ?
				"does not contain a `stages` line." :
				"contains too many `stages` lines. Please provide only one."
		);

		lise_obj_format_free(&loaded_format);
		return false;
	}

	if (found_lines[0]->token_count == 0)
	{
		LERROR(
			"Provided config file `%s` contains a `stages` line but no stages were specified."
			"Please specify one or more.",
			path
		);

		lise_obj_format_free(&loaded_format);
		return false;
	}

	out_config->stage_count = found_lines[0]->token_count;
	out_config->stage_names = malloc(out_config->stage_count * sizeof(char*));
	
	for (uint64_t i = 0; i < out_config->stage_count; i++)
	{
		out_config->stage_names[i] = strdup(found_lines[0]->tokens[i]);
	}

	// Load stage file paths.
	lise_obj_format_get_line(&loaded_format, "stage_files", &found_line_count, &found_lines);

	if (found_line_count != 1)
	{
		LERROR("Provided config file `%s` %s",
			path,
			found_line_count == 0 ?
				"does not contain a `stage_files` line." :
				"contains too many `stage_files` lines. Please provide only one."
		);

		lise_obj_format_free(&loaded_format);
		return false;
	}

	if (found_lines[0]->token_count == 0)
	{
		LERROR(
			"Provided config file `%s` contains a `stage_files` line but no stage file paths were specified."
			"Please specify one or more.",
			path
		);

		lise_obj_format_free(&loaded_format);
		return false;
	}

	if (found_lines[0]->token_count != out_config->stage_count)
	{
		LERROR(
			"The amount of provided stage file paths does not math the amount of provided stage names "
			"in config file `%s`.",
			path
		);

		lise_obj_format_free(&loaded_format);
		return false;
	}

	out_config->stage_file_names = malloc(out_config->stage_count * sizeof(char*));

	for (uint32_t i = 0; i < out_config->stage_count; i++)
	{
		out_config->stage_file_names[i] = strdup(found_lines[0]->tokens[i]);
	}

	// Load attributes
	lise_obj_format_get_line(&loaded_format, "attribute", &found_line_count, &found_lines);

	if (found_line_count == 0)
	{
		LERROR("Provided config file `%s` does not contain any `attribute` lines. Please provide one or more.", path);
		
		lise_obj_format_free(&loaded_format);
		return false;
	}

	blib_darray attributes = blib_darray_create(lise_shader_config_attribute);

	for (uint64_t i = 0; i < found_line_count; i++)
	{
		if (found_lines[i]->token_count != 2)
		{
			LERROR(
				"An `attribute` line in config file `%s` does not contain exactly two (2) parameters. `attribute` lines"
				" need to contain exactly two (2) parameters; a type and a name.",
				path
			);

			blib_darray_free(&attributes);
			lise_obj_format_free(&loaded_format);
			return false;
		}

		lise_shader_config_attribute attribute;
		attribute.type = strdup(found_lines[i]->tokens[0]);
		attribute.name = strdup(found_lines[i]->tokens[1]);

		blib_darray_push_back(&attributes, &attribute);
	}

	out_config->attribute_count = attributes.size;

	uint64_t attributes_size = out_config->attribute_count * sizeof(lise_shader_config_attribute);
	out_config->attributes = malloc(attributes_size);

	memcpy(out_config->attributes, blib_darray_get(&attributes, 0), attributes_size);

	blib_darray_free(&attributes);

	// Load uniforms
	lise_obj_format_get_line(&loaded_format, "uniform", &found_line_count, &found_lines);

	if (found_line_count == 0)
	{
		LERROR("Provided config file `%s` does not contain any `uniform` lines. Please provide one or more.", path);
		
		lise_obj_format_free(&loaded_format);
		return false;
	}

	blib_darray uniforms = blib_darray_create(lise_shader_config_uniform);

	for (uint64_t i = 0; i < found_line_count; i++)
	{
		if (found_lines[i]->token_count != 3)
		{
			LERROR(
				"A `uniform` line in config file `%s` does not contain exactly three (3) parameters. `uniform` lines "
				"need to contain exactly three (3) parameters; a type, a scope and a name.",
				path
			);

			blib_darray_free(&uniforms);
			lise_obj_format_free(&loaded_format);
			return false;
		}

		lise_shader_config_uniform uniform;
		uniform.type = strdup(found_lines[i]->tokens[0]);
		uniform.scope = strtoul(found_lines[i]->tokens[1], NULL, 10);
		uniform.name = strdup(found_lines[i]->tokens[2]);

		blib_darray_push_back(&uniforms, &uniform);
	}

	out_config->uniform_count = uniforms.size;

	uint64_t uniforms_size = out_config->uniform_count * sizeof(lise_shader_config_uniform);
	out_config->uniforms = malloc(uniforms_size);

	memcpy(out_config->uniforms, blib_darray_get(&uniforms, 0), uniforms_size);

	blib_darray_free(&attributes);
	blib_darray_free(&uniforms);

	// Cleanup
	lise_obj_format_free(&loaded_format);

	return true;
}

void lise_shader_config_free(lise_shader_config* config)
{
	for (uint64_t i = 0; i < config->uniform_count; i++)
	{
		free(config->uniforms[i].name);
		free(config->uniforms[i].type);
	}

	free(config->uniforms);

	for (uint64_t i = 0; i < config->attribute_count; i++)
	{
		free(config->attributes[i].name);
		free(config->attributes[i].type);
	}
	
	free(config->attributes);

	for (uint64_t i = 0; i < config->stage_count; i++)
	{
		free(config->stage_names[i]);
		free(config->stage_file_names[i]);
	}

	free(config->stage_names);
	free(config->stage_file_names);

	free(config->name);
}
