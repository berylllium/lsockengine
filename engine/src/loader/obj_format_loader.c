#include "loader/obj_format_loader.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "core/logger.h"
#include "container/darray.h"

#include "platform/filesystem.h"

#define COMMENT_CHAR '#'

bool lise_obj_format_load(const char* path, lise_obj_format* out_obj_format)
{
	lise_file_handle fh;

	if (!lise_filesystem_open(path, LISE_FILE_MODE_READ, false, &fh))
	{
		LERROR("The obj formatter failed to open the following file: `%s`.", path);
		return false;
	}

	// Create a temporary dynamic array to store all the obj_format_lines.
	blib_darray tokenized_lines = blib_darray_create(lise_obj_format_line, 8);

	uint64_t line_length;
	char line[1024];
	while (lise_filesystem_read_line(&fh, &line_length, line))
	{
		bool is_comment_line = false;

		// Empty line; skip.
		if (line_length == 0) continue;

		// Allocate a temprary dynamic array to store the pointers to the tokens.
		blib_darray tokens =  blib_darray_create(char*, 4);

		// Reset line_length so that it can be used in order to calculate the effective length of the line. The
		// effective length of the line is how long the line is, excluding any comments.
		line_length = 0;

		// Iterate tokens.
		char* token = strtok(line, " \t");
		while (token)
		{
			if (token[0] == COMMENT_CHAR)
			{
				// The token starts with a comment character.

				if (tokens.size == 0)
				{
					// The first token in the line is a comment; the entire line is a comment; skip line.
					is_comment_line = true;
					break;
				}
				else
				{
					// This token and any token from here on is a comment; skip this, and all further tokens.
					break;
				}
			}

			line_length += strlen(token) + 1; // Add one to the value to include the null-terminator.

			blib_darray_push_back(&tokens, &token);

			token = strtok(NULL, " \t");
		}

		// The line is a comment; skip.
		if (is_comment_line) 
		{
			blib_darray_free(&tokens);
			continue;
		}

		lise_obj_format_line obj_format_line;
		obj_format_line.token_count = tokens.size - 1; // Subtract one because the type token doesn't count.

		// Allocate the tokens array.
		obj_format_line.tokens = malloc(obj_format_line.token_count * sizeof(char*));

		// Allocate the line.
		obj_format_line.line = malloc(line_length);

		// The line will now be a null-terminator seperated list of strings because of how strtok works. Iterate through
		// these tokens and populate the lise_obj_formate_line structure.
		uint64_t offset = 0;
		for (uint64_t i = 0; i < tokens.size; i++)
		{
			token = *((char**) blib_darray_get(&tokens, i));

			uint64_t token_len = strlen(token);

			// Copy over the token to the output line.
			memcpy(obj_format_line.line + offset, token, token_len + 1);

			// Token is the first token in the list; this is the type.
			if (i == 0)
			{
				obj_format_line.type = obj_format_line.line;
			}
			else
			{
				obj_format_line.tokens[i - 1] = obj_format_line.line + offset;
			}

			offset += token_len + 1;
		}

		blib_darray_push_back(&tokenized_lines, &obj_format_line);

		blib_darray_free(&tokens);
	}

	// Populate the out_obj_format.
	out_obj_format->line_count = tokenized_lines.size;

	uint64_t arr_len = tokenized_lines.size * sizeof(lise_obj_format_line);
	out_obj_format->lines = malloc(arr_len);
	memcpy(out_obj_format->lines, blib_darray_get(&tokenized_lines, 0), arr_len);

	blib_darray_free(&tokenized_lines);

	return true;
}

void lise_obj_format_free(lise_obj_format* obj_format)
{
	// Free lines.
	for (uint64_t i = 0; i < obj_format->line_count; i++)
	{
		free(obj_format->lines[i].tokens);
		free(obj_format->lines[i].line);
	}

	free(obj_format->lines);
}

bool lise_obj_format_get_line(
	lise_obj_format* obj_format,
	const char* type, 
	uint64_t* out_found_line_count, 
	lise_obj_format_line*** out_found_lines
)
{
	blib_darray found_lines = blib_darray_create(lise_obj_format_line*);

	// Iterate through all te lines.
	for (uint64_t i = 0; i < obj_format->line_count; i++)
	{
		if (strcmp(type, obj_format->lines[i].type) == 0)
		{
			// The types match.
			lise_obj_format_line* line_addr = &obj_format->lines[i];
			blib_darray_push_back(&found_lines, &line_addr);
		}
	}

	if (found_lines.size == 0)
	{
		// No matches were found.
		*out_found_line_count = 0;
		*out_found_lines = NULL;

		blib_darray_free(&found_lines);
		return false;
	}

	// At least one match was found.
	*out_found_line_count = found_lines.size;
	
	// Allocate a new list and copy over the pointers.
	*out_found_lines = blib_darray_copy_data(&found_lines);


	blib_darray_free(&found_lines);

	return true;
}
