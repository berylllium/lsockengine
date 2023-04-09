/**
 * @file obj_format_loader.h
 * @brief This header file contains definitions of functions and structures relating to loading the obj format.
 * 
 * The term "obj format" refers to any file containing configurations in the
 * <a href="https://en.wikipedia.org/wiki/Wavefront_.obj_file" target="_blank">Wavefront obj format</a>, and not just
 * geometry data. An example of this is the shader configurations, which also use the "obj format" to store shader
 * configurations.
 */
#pragma once

#include "definitions.h"

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
typedef struct lise_obj_format_line
{
	/**
	 * @brief The "type" of the line. The type is the first token in a line.
	 */
	const char* type;

	/**
	 * @brief The amount of tokens pointed to by the \ref tokens member.
	 */
	uint64_t token_count;

	/**
	 * @brief An array of pointers that point to the tokens within the allocation pointed to by the \ref line member.
	 */
	char** tokens;

	/**
	 * @brief Contains the actual line.
	 * 
	 * The \ref type and \ref tokens pointers all point to somewhere within the allocation
	 * pointed to by this pointer. This is also the only pointer that needs to be freed, even though it is not
	 * recommended to do so manually; the ::lise_obj_format_free function already frees all the
	 * \ref lise_obj_format_line s pointed to by the \ref lise_obj_format.
	 */
	char* line;
} lise_obj_format_line;

/**
 * @brief Represents the entire obj format file.
 */
typedef struct lise_obj_format
{
	/**
	 * @brief The amount of lines pointed to by the \ref lines member.
	 */
	uint64_t line_count;

	/**
	 * @brief An array of \ref lise_obj_format_line s.
	 */
	lise_obj_format_line* lines;
} lise_obj_format;

/**
 * @brief Tries to load and parse an obj format file.
 * 
 * @param path The path to the obj format file. This can be relative or absolute.
 * @param out_obj_format [out] A pointer to where to load the obj format.
 * @return true if the file was successfully loaded and parsed.
 * @return false if there was an error loading or parsing the file.
 */
bool lise_obj_format_load(const char* path, lise_obj_format* out_obj_format);

/**
 * @brief Frees a \ref lise_obj_format object.
 * 
 * The function will also free all the \ref lise_obj_format_line s pointed to by the \ref lise_obj_format object.
 * 
 * @param obj_format The \ref lise_obj_format object to free.
 */
void lise_obj_format_free(lise_obj_format* obj_format);

/**
 * @brief Tries to find the provided type of line. If both parent_type and parent_token are not NULL, then the tokens
 * must be preceded by a line with the provided type and first token.
 * 
 * @param obj_format The format struct to search.
 * @param type The type of line to get.
 * @param parent_type The type of the parent line. Use NULL if you do not want to use this feature.
 * @param parent_token The first token of the parent line. Use NULL if you do not want to use this feature.
 * @param out_found_line_count [out] The amount of found lines.
 * @param out_found_lines [out] An array of pointers to the found lines. This array gets allocated in the function.
 * BE SURE TO FREE IT!
 * 
 * @return true if at least one line was found.
 * @return false if no lines were found.
 */
bool lise_obj_format_get_line(
	lise_obj_format* obj_format,
	const char* type, 
	const char* parent_type,
	const char* parent_token,
	uint64_t* out_found_line_count, 
	lise_obj_format_line*** out_found_lines
);
