#pragma once

#include "definitions.h"

/**
 * @brief A wrapper for the std FILE handle.
 */
typedef struct lise_file_handle
{
	void* handle;
	bool is_valid;
} lise_file_handle;

typedef enum lise_file_modes
{
	LISE_FILE_MODE_READ = 	0b01,
	LISE_FILE_MODE_WRITE =	0b10
} lise_file_modes;

/**
 * @brief Checks whether the given file exists.
 * 
 * @param path The path to check.
 * @return bool	Returns true if the file was found, false if it was not.
 */
LAPI bool lise_filesystem_exists(const char* path);

/**
 * @brief Opens the given file.
 * 
 * @param path The path to the file to open.
 * @param mode The mode in which to open the file. This can be a combination of modes.
 * @param binary Whether to interpret the bytes within the file as text or binary data.
 * @param out_file_handle A pointer to the file handle structure in which to output the opened file.
 * @return bool Whether the function returned successfully or not. 
 */
LAPI bool lise_filesystem_open(const char* path, lise_file_modes mode, bool binary, lise_file_handle* out_file_handle);

LAPI void lise_filesystem_close(lise_file_handle* file_handle);

LAPI bool lise_filesystem_read_line(lise_file_handle* file_handle, uint64_t* out_read_bytes, char** out_line_buff);

LAPI bool lise_filesystem_write_line(lise_file_handle* file_handle, char* s);

LAPI bool lise_filesystem_write(lise_file_handle* file_handle, char* s, uint64_t* out_bytes_written);

LAPI bool lise_filesystem_read(
	lise_file_handle* file_handle,
	uint64_t data_size,
	uint64_t* out_bytes_read,
	char* out_data_buff
);

LAPI bool lise_filesystem_read_all(lise_file_handle* file_handle, uint64_t* out_read_bytes, char** out_data_buff);
