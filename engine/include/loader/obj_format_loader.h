#pragma once

#include "definitions.h"

typedef struct lise_obj_format_line
{
	const char* type;

	uint64_t token_count;
	char** tokens;

	// Contains the actual line. The type and tokens pointers all point to somewhere within the data pointed to by the
	// line pointer. This is also the only pointer that needs to be freed.
	char* line;
} lise_obj_format_line;

typedef struct lise_obj_format
{
	uint64_t line_count;
	lise_obj_format_line* lines;
} lise_obj_format;

bool lise_obj_format_load(const char* path, lise_obj_format* out_obj_format);

void lise_obj_format_free(lise_obj_format* obj_format);

/**
 * @brief Tries to find the provided type of line. If both parent_type and parent_token are not NULL, then the tokens
 * must be preceded by a line with the provided type and first token.
 * 
 * @param obj_format The format struct to search.
 * @param type The type of line to get.
 * @param parent_type The type of the parent line. Use NULL if you do not want to use this feature.
 * @param parent_token The first token of the parent line. Use NULL if you do not want to use this feature.
 * @param out_found_line_count [OUT] The amount of found lines.
 * @param out_found_lines [OUT] An array of pointers to the found lines. This array gets allocated in the function.
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
