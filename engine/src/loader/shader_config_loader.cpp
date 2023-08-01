#include "loader/shader_config_loader.hpp"

#include <simple-logger.hpp>

#include "loader/obj_format_loader.hpp"

namespace lise
{

bool shader_config_load(const std::string& path, ShaderConfig& out_config)
{
	ObjFormat loaded_format;

	if (!obj_format_load(path, loaded_format))
	{
		sl::log_error("Failed to load shader config for shader: `{}`.", path);
		return false;
	}

	// Get the name.
	std::vector<const ObjFormatLine*> found_lines = obj_format_get_line(loaded_format, "name");

	if (found_lines.size() != 1)
	{
		sl::log_error("Provided config file `{}` {}",
			path,
			found_lines.size() == 0 ?
				"does not contain a `name` line." :
				"contains too many `name` lines. Please provide only one."
		);

		return false;
	}

	if (found_lines[0]->tokens.size() != 1)
	{
		sl::log_error("Provided config file `{}` contains a `name` line, {}",
			path,
			found_lines[0]->tokens.size() == 0 ?
				"but no names were specified. Please specify one." :
				"but too many names were specified. Please specify only one."
		);

		return false;
	}

	out_config.name = found_lines[0]->tokens[0];

	// Get the render pass name.
	found_lines = obj_format_get_line(loaded_format, "render_pass");

	if (found_lines.size() != 1)
	{
		sl::log_error("Provided config file `{}` {}",
			path,
			found_lines.size() == 0 ?
				"does not contain a `render_pass` line." :
				"contains too many `render_pass` lines. Please provide only one."
		);

		return false;
	}

	if (found_lines[0]->tokens.size() != 1)
	{
		sl::log_error("Provided config file `{}` contains a `render_pass` line, {}",
			path,
			found_lines[0]->tokens.size() == 0 ?
				"but no render pass names were specified. Please specify one." :
				"but too many render pass names were specified. Please specify only one."
		);

		return false;
	}

	out_config.render_pass_name = found_lines[0]->tokens[0];

	// Load stage names.
	found_lines = obj_format_get_line(loaded_format, "stages");

	if (found_lines.size() != 1)
	{
		sl::log_error("Provided config file `{}` {}",
			path,
			found_lines.size() == 0 ?
				"does not contain a `stages` line." :
				"contains too many `stages` lines. Please provide only one."
		);

		return false;
	}

	if (found_lines[0]->tokens.size() == 0)
	{
		sl::log_error(
			"Provided config file `{}` contains a `stages` line but no stages were specified."
			"Please specify one or more.",
			path
		);

		return false;
	}

	out_config.stage_names = found_lines[0]->tokens;
	
	// Load stage file paths.
	found_lines = obj_format_get_line(loaded_format, "stage_files");

	if (found_lines.size() != 1)
	{
		sl::log_error("Provided config file `{}` {}",
			path,
			found_lines.size() == 0 ?
				"does not contain a `stage_files` line." :
				"contains too many `stage_files` lines. Please provide only one."
		);

		return false;
	}

	if (found_lines[0]->tokens.size() == 0)
	{
		sl::log_error(
			"Provided config file `{}` contains a `stage_files` line but no stage file paths were specified."
			"Please specify one or more.",
			path
		);

		return false;
	}

	if (found_lines[0]->tokens.size() != out_config.stage_names.size())
	{
		sl::log_error(
			"The amount of provided stage file paths does not math the amount of provided stage names "
			"in config file `{}`.",
			path
		);

		return false;
	}

	out_config.stage_file_names = found_lines[0]->tokens;

	// Load attributes
	found_lines = obj_format_get_line(loaded_format, "attribute");

	if (found_lines.size() == 0)
	{
		sl::log_error("Provided config file `{}` does not contain any `attribute` lines. Please provide one or more.", path);
		
		return false;
	}

	out_config.attributes.resize(found_lines.size());

	for (uint64_t i = 0; i < out_config.attributes.size(); i++)
	{
		if (found_lines[i]->tokens.size() != 2)
		{
			sl::log_error(
				"An `attribute` line in config file `{}` does not contain exactly two (2) parameters. `attribute` lines"
				" need to contain exactly two (2) parameters; a type and a name.",
				path
			);

			return false;
		}

		out_config.attributes[i].type = found_lines[i]->tokens[0];
		out_config.attributes[i].name = found_lines[i]->tokens[1];
	}

	// Load uniforms
	found_lines = obj_format_get_line(loaded_format, "uniform");

	if (found_lines.size() == 0)
	{
		sl::log_error("Provided config file `{}` does not contain any `uniform` lines. Please provide one or more.", path);
		
		return false;
	}

	out_config.uniforms.resize(found_lines.size());

	for (uint64_t i = 0; i < out_config.uniforms.size(); i++)
	{
		if (found_lines[i]->tokens.size() != 3)
		{
			sl::log_error(
				"A `uniform` line in config file `{}` does not contain exactly three (3) parameters. `uniform` lines "
				"need to contain exactly three (3) parameters; a type, a scope and a name.",
				path
			);

			return false;
		}

		out_config.uniforms[i].type = found_lines[i]->tokens[0];
		out_config.uniforms[i].scope = strtoul(found_lines[i]->tokens[1].c_str(), NULL, 10);
		out_config.uniforms[i].name = found_lines[i]->tokens[2];
	}

	return true;
}

}
