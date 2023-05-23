/**
 * @file shader_config_loader.hpp
 * @brief This header file contains definitions of structures and functions relating to loading shader configurations.
 */
#pragma once

#include <string>
#include <memory>

#include "definitions.hpp"

namespace lise
{

/**
 * @brief A structure that contains attribute configurations.
 * 
 * This attribute is usually a vertex attribute.
 */
struct ShaderConfigAttribute
{
	/**
	 * @brief The type of the attribute.
	 */
	std::string type;

	/**
	 * @brief The name of the attribute.
	 */
	std::string name;
};

/**
 * @brief A structure that contains uniform configurations.
 */
struct ShaderConfigUniform
{
	/**
	 * @brief The type of the uniform.
	 */
	std::string type;

	/**
	 * @brief The scope of the uniform.
	 */
	uint32_t scope;

	/**
	 * @brief The name of the uniform.
	 */
	std::string name;
};

/**
 * @brief A structure that contains shader configurations.
 */
struct ShaderConfig
{
	/**
	 * @brief The name of the shader.
	 */
	std::string name;

	/**
	 * @brief The amount of shader stages that the shader has.
	 */
	uint32_t stage_count;

	/**
	 * @brief An array of pointers that point to strings representing the names of the stages.
	 * 
	 * There are \ref stage_count amount of pointers in the array.
	 */
	std::unique_ptr<std::string[]> stage_names;

	/**
	 * @brief An array of pointers that point to strings representing the relative paths to the SPIR-V shader module
	 * files.
	 * 
	 * There are \ref stage_count amount of pointers in the array.
	 */
	std::unique_ptr<std::string[]> stage_file_names;

	/**
	 * @brief The name of the render pass.
	 */
	std::string render_pass_name;

	/**
	 * @brief The amount of attributes stored in the allocation pointed to by the \ref attributes member.
	 */
	uint32_t attribute_count;

	/**
	 * @brief An array of attributes.
	 * 
	 * The allocation is owned by the \ref lise_shader_config object.
	 */
	std::unique_ptr<ShaderConfigAttribute[]> attributes;

	/**
	 * @brief The amount of uniforms stored in the allocation pointed to by the \ref uniforms member.
	 */
	uint32_t uniform_count;

	/**
	 * @brief An array of uniforms.
	 * 
	 * The allocation is owned by the \ref lise_shader_config object.
	 */
	std::unique_ptr<ShaderConfigUniform[]> uniforms;
};

/**
 * @brief Attempts to load the shader config file.
 * 
 * @param path The path to the shader config file. Can be relative or absolute.
 * @param out_config [out] A pointer to where to load the shader config to.
 * @return true if the shader config file was successfully loaded and paresed.
 * @return false if there was an error loading or parsing the shader config file.
 */
bool shader_config_load(const std::string& path, ShaderConfig& out_config);

}
