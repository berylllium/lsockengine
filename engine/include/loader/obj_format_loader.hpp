/**
 * @file obj_format_loader.hpp
 * @brief This header file contains definitions of functions and structures relating to loading the obj format.
 * 
 * The term "obj format" refers to any file containing configurations in the
 * <a href="https://en.wikipedia.org/wiki/Wavefront_.obj_file" target="_blank">Wavefront obj format</a>, and not just
 * geometry data. An example of this is the shader configurations, which also use the "obj format" to store shader
 * configurations.
 */
#pragma once

#include <string>
#include <string_view>
#include <memory>
#include <vector>

#include "definitions.hpp"

namespace lise
{

/**
 * @brief Represents a "line" of an obj format file.
 * 
 * Lines are seperated by new lines and made up of tokens. Tokens are groups of characters seperated by white space.
 * See the following example: A line consists of four (4) tokens: `this is an example.`, these are `this`, `is`, `an`,
 * `example.`.
 * 
 * The struct consists of multiple pointers but has only one memory allocation, that being the #line member. All other
 * pointers point to a location within the allocation pointed to by the #line member.
 */
struct ObjFormatLine
{
	/**
	 * @brief The "type" of the line. The type is the first token in a line.
	 */
	std::string type;
	/**
	 * @brief The amount of tokens pointed to by the \ref tokens member.
	 */
	uint64_t token_count;

	/**
	 * @brief An array of pointers that point to the tokens within the allocation pointed to by the \ref line member.
	 */
	std::unique_ptr<std::string[]> tokens;
};

/**
 * @brief Represents the entire obj format file.
 */
struct ObjFormat
{
	/**
	 * @brief The amount of lines pointed to by the \ref lines member.
	 */
	uint64_t line_count;

	/**
	 * @brief An array of \ref lise_obj_format_line s.
	 */
	std::unique_ptr<ObjFormatLine[]> lines;
};

/**
 * @brief Tries to load and parse an obj format file.
 * 
 * @param path The path to the obj format file. This can be relative or absolute.
 * @param out_obj_format [out] A pointer to where to load the obj format.
 * @return true if the file was successfully loaded and parsed.
 * @return false if there was an error loading or parsing the file.
 */
bool obj_format_load(const std::string& path, ObjFormat& out_obj_format);

/**
 * @brief Tries to find the provided type of line. If both parent_type and parent_token are not NULL, then the tokens
 * must be preceded by a line with the provided type and first token.
 * 
 * @param obj_format The format struct to search.
 * @param type The type of line to get.
 * @param parent_type The type of the parent line.
 * @param parent_token The first token of the parent line.
 * 
 * @return A vector of pointers to the found lines.
 */
std::vector<ObjFormatLine*> obj_format_get_line(
	const ObjFormat& obj_format,
	const std::string& type, 
	const std::string& parent_type,
	const std::string& parent_token
);

std::vector<ObjFormatLine*> obj_format_get_line(const ObjFormat& obj_format, const std::string& type);

}
