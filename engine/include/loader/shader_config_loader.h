/**
 * @file shader_config_loader.h
 * @brief This header file contains definitions of structures and functions relating to loading shader configurations.
 */
#pragma once

#include "definitions.h"

/**
 * @brief A structure that contains attribute configurations.
 * 
 * This attribute is usually a vertex attribute.
 */
typedef struct lise_shader_config_attribute
{
	/**
	 * @brief The type of the attribute.
	 * 
	 * The allocation is owned by the \ref lise_shader_config_attribute object.
	 */
	char* type;

	/**
	 * @brief The name of the attribute.
	 * 
	 * The allocation is owned by the \ref lise_shader_config_attribute object.
	 */
	char* name;
} lise_shader_config_attribute;

/**
 * @brief A structure that contains uniform configurations.
 */
typedef struct lise_shader_config_uniform
{
	/**
	 * @brief The type of the uniform.
	 * 
	 * The allocation is owned by the \ref lise_shader_config_uniform object.
	 */
	char* type;

	/**
	 * @brief The scope of the uniform.
	 */
	uint32_t scope;

	/**
	 * @brief The name of the uniform.
	 * 
	 * The allocation is owned by the \ref lise_shader_config_uniform object.
	 */
	char* name;
} lise_shader_config_uniform;

/**
 * @brief A structure that contains shader configurations.
 */
typedef struct lise_shader_config
{
	/**
	 * @brief The name of the shader.
	 * 
	 * The allocation is owned by the \ref lise_shader_config object.
	 */
	char* name;

	/**
	 * @brief The amount of shader stages that the shader has.
	 */
	uint32_t stage_count;

	/**
	 * @brief An array of pointers that point to strings representing the names of the stages.
	 * 
	 * There are \ref stage_count amount of pointers in the array.
	 * 
	 * The allocation is owned by the \ref lise_shader_config object.
	 */
	char** stage_names;

	/**
	 * @brief An array of pointers that point to strings representing the relative paths to the SPIR-V shader module
	 * files.
	 * 
	 * There are \ref stage_count amount of pointers in the array.
	 * 
	 * The allocation is owned by the \ref lise_shader_config object.
	 */
	char** stage_file_names;

	/**
	 * @brief The name of the render pass.
	 * 
	 * The allocation is owned by the \ref lise_shader_config object.
	 */
	char* render_pass_name;

	/**
	 * @brief The amount of attributes stored in the allocation pointed to by the \ref attributes member.
	 */
	uint32_t attribute_count;

	/**
	 * @brief An array of attributes.
	 * 
	 * The allocation is owned by the \ref lise_shader_config object.
	 */
	lise_shader_config_attribute* attributes;

	/**
	 * @brief The amount of uniforms stored in the allocation pointed to by the \ref uniforms member.
	 */
	uint32_t uniform_count;

	/**
	 * @brief An array of uniforms.
	 * 
	 * The allocation is owned by the \ref lise_shader_config object.
	 */
	lise_shader_config_uniform* uniforms;
} lise_shader_config;

/**
 * @brief Attempts to load the shader config file.
 * 
 * @param path The path to the shader config file. Can be relative or absolute.
 * @param out_config [out] A pointer to where to load the shader config to.
 * @return true if the shader config file was successfully loaded and paresed.
 * @return false if there was an error loading or parsing the shader config file.
 */
bool lise_shader_config_load(const char* path, lise_shader_config* out_config);

/**
 * @brief Frees a \ref lise_shader_config object.
 * 
 * @param config The \ref lise_shader_config object to free.
 */
void lise_shader_config_free(lise_shader_config* config);
