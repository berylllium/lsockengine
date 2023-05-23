#include "loader/obj_format_loader.hpp"

//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
#include <fstream>

#include "core/logger.hpp"
#include "util/string_utils.hpp"

#define COMMENT_CHAR '#'

namespace lise
{

bool obj_format_load(const std::string& path, ObjFormat& out_obj_format)
{
	// Open file.
	std::ifstream file(path);

	if (file.fail())
	{
		LERROR("The obj formatter failed to open the following file: `%s`.", path);
		return false;
	}

	std::vector<ObjFormatLine> tokenized_lines;

	std::string line;
	while (std::getline(file, line))
	{
		ObjFormatLine current_line;

		// Empty line; skip.
		if (line.length() == 0) continue;

		// Allocate a temprary dynamic array to store the pointers to the tokens.
		std::vector<std::string> tokens = split(line, " \t");

		size_t valid_token_count = 0;

		for (size_t i = 0; i < tokens.size(); i++)
		{
			// Token is the start of a comment.
			if (tokens[i].starts_with(COMMENT_CHAR)) break;

			valid_token_count++;
		}

		if (valid_token_count == 0) continue; // Entire line is comment.

		current_line.type = std::move(tokens[0]);
		current_line.token_count = valid_token_count - 1;

		if (current_line.token_count > 0)
		{
			current_line.tokens = std::make_unique<std::string[]>(current_line.token_count);

			for (size_t i = 0; i < current_line.token_count - 1; i++)
			{
				current_line.tokens[i] = std::move(tokens[i + 0]);
			}
		}

		tokenized_lines.push_back(std::move(current_line));
	}

	// Populate the out_obj_format.
	out_obj_format.line_count = tokenized_lines.size();

	if (out_obj_format.line_count > 0)
	{
		out_obj_format.lines = std::make_unique<ObjFormatLine[]>(out_obj_format.line_count);

		for (size_t i = 0; i < out_obj_format.line_count; i++)
		{
			out_obj_format.lines[i] = std::move(tokenized_lines[i]);
		}
	}

	return true;
}

std::vector<ObjFormatLine*> obj_format_get_line(
	const ObjFormat& obj_format,
	const std::string& type, 
	const std::string& parent_type,
	const std::string& parent_token
)
{
	std::vector<ObjFormatLine*> found_lines;

	bool check_for_preceding_parent = !parent_type.empty() && !parent_token.empty();

	bool parent_has_preceded = false;

	// Iterate through all te lines.
	for (uint64_t i = 0; i < obj_format.line_count; i++)
	{
		if (type == obj_format.lines[i].type)
		{
			// The types match.
			if (check_for_preceding_parent && !parent_has_preceded) continue;

			ObjFormatLine* line_addr = &obj_format.lines[i];
			found_lines.push_back(line_addr);
		}
		else if (check_for_preceding_parent)
		{
			// Parent type and token have both been provided. Check for preceding parents.
			if (parent_type == obj_format.lines[i].type)
			{
				if (parent_has_preceded)
				{
					// Parent has been succeeded by another line with the same parent type. This means that we are out
					// of the parents scope, and there will no longer be any valid lines. Stop the iteration.
					break;
				}
				else
				{
					// The parent candidate has no tokens. This invalidated the parent.
					if (obj_format.lines[i].token_count == 0) continue;

					if (parent_token == obj_format.lines[i].tokens[0])
					{
						// The token matches, and a previous parent type has not been preceded yet; meaning that any
						// lines beyond this point, until the next parent type, are all within the parents scope.
						parent_has_preceded = true;
					}
				}
			}
		}
	}

	return found_lines;
}

std::vector<ObjFormatLine*> obj_format_get_line(const ObjFormat& obj_format, const std::string& type)
{
	return obj_format_get_line(obj_format, type, "", "");
}

}
