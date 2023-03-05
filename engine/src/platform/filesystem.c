#include "platform/filesystem.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "core/logger.h"

LAPI bool lise_filesystem_exists(const char* path)
{
	struct stat buffer;
	return stat(path, &buffer) == 0;
}

LAPI bool lise_filesystem_open(const char* path, lise_file_modes mode, bool binary, lise_file_handle* out_file_handle)
{
	out_file_handle->is_valid = false;
	out_file_handle->handle = NULL;

	const char* mode_str;

	if ((mode & LISE_FILE_MODE_READ) && (mode & LISE_FILE_MODE_WRITE))
	{
		mode_str = binary ? "w+b" : "w+";
	}
	else if ((mode & LISE_FILE_MODE_READ) && !(mode & LISE_FILE_MODE_WRITE))
	{
		mode_str = binary ? "rb" : "r";
	}
	else if (!(mode & LISE_FILE_MODE_READ) && (mode & LISE_FILE_MODE_WRITE))
	{
		mode_str = binary ? "wb" : "w";
	}
	else
	{
		LERROR("Invalid mode passed while trying to open file `%s`.", path);
		return false;
	}

	FILE* file = fopen(path, mode_str);
	if (!file)
	{
		LERROR("Failed to open file `%s`.", path);
		return false;
	}

	out_file_handle->handle = file;
	out_file_handle->is_valid = true;

	return true;
}

LAPI void lise_filesystem_close(lise_file_handle* file_handle)
{
	if (file_handle->handle)
	{
		fclose(file_handle->handle);

		file_handle->handle = NULL;
		file_handle->is_valid = false;
	}
}

LAPI bool lise_filesystem_read_line(lise_file_handle* file_handle, uint64_t* out_read_bytes, char** out_line_buff)
{
	uint64_t begin_pos = ftell(file_handle->handle);

	// Figure out of long the line is
	uint64_t line_length = 0;

	while (true)
	{
		char c = fgetc(file_handle->handle);

		if (c == EOF || c == '\n') break;

		line_length++;
	}

	fseek(file_handle->handle, -line_length, SEEK_CUR);

	*out_line_buff = malloc(line_length * sizeof(char) + 1); // +1 for null termination

	fread(*out_line_buff, sizeof(char), line_length, file_handle->handle);

	(*out_line_buff)[line_length] = '\0';

	*out_read_bytes = line_length;

	return true;
}

LAPI bool lise_filesystem_write_line(lise_file_handle* file_handle, char* s)
{
	if (fputs(s, file_handle->handle) != EOF)
	{
		fputc('\n', file_handle->handle);

		// Flush to prevent data loss on crashes.
		fflush(file_handle->handle);

		return true;
	}

	LERROR("Failed to write to file.");

	return false;
}

LAPI bool lise_filesystem_write(lise_file_handle* file_handle, char* s, uint64_t* out_bytes_written)
{
	uint64_t slen = strlen(s);

	*out_bytes_written = fwrite(s, slen, strlen(s), file_handle->handle);

	if (*out_bytes_written != slen)
	{
		LERROR("Amount of written bytes does not match amount of provided bytes during writing to file");
		return false;
	}

	// Flush to prevent data loss on crashes
	fflush(file_handle->handle);

	return true;
}

LAPI bool lise_filesystem_read(
	lise_file_handle* file_handle,
	uint64_t data_size,
	uint64_t* out_bytes_read,
	char* out_data_buff
)
{
	*out_bytes_read = fread(out_data_buff, sizeof(char), data_size, file_handle->handle);

	if (*out_bytes_read != data_size)
	{
		LERROR("Amount of read bytes does not match the amount of provided bytes during reading of file");
		return false;
	}

	return true;
}

LAPI bool lise_filesystem_read_all(lise_file_handle* file_handle, uint64_t* out_read_bytes, char** out_data_buff)
{
	fseek(file_handle->handle, 0, SEEK_END);

	uint64_t size = ftell(file_handle->handle);

	rewind(file_handle->handle);

	*out_data_buff = malloc(size * sizeof(char) + 1); // +1 for null termination
	(*out_data_buff)[size] = '\0';

	*out_read_bytes = fread(*out_data_buff, sizeof(char), size, file_handle->handle);

	if (*out_read_bytes != size)
	{
		LERROR("Amount of read bytes does not match the amount of provided bytes during reading of file");
		return false;
	}

	return true;
}
