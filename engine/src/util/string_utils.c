#include "util/string_utils.h"

#include <string.h>

bool lise_util_string_array_contains(const char** arr, uint64_t arr_length, const char* comp)
{
	for (uint64_t i = 0; i < arr_length; i++)
	{
		if (strcmp(arr[i], comp) == 0)
			return true;
	}

	return false;
}
