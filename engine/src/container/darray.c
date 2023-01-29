#include "container/darray.h"

#include <stdlib.h>
#include <string.h>

lise_darray _lise_darray_create(uint64_t default_capacity, uint64_t stride)
{
	lise_darray darray = {};

	darray.capacity = default_capacity;
	darray.stride = stride;

	darray.data = calloc(1, darray.capacity * darray.stride);

	return darray;
}

void* lise_darray_get(lise_darray* darray, uint64_t index)
{
	return darray->data + darray->stride * index;
}

void lise_darray_push_back(lise_darray* darray, void* element)
{
	if (darray->size + 1 > darray->capacity)
	{
		lise_darray_resize(darray, darray->capacity * LISE_DARRAY_RESIZE_FACTOR);
	}

	memcpy(darray->data + darray->size * darray->stride, element, darray->stride);

	darray->size++;
}

void lise_darray_resize(lise_darray* darray, uint64_t new_capacity)
{
	darray->capacity = new_capacity;

	void* temp = realloc(darray->data, darray->stride * new_capacity);

	darray->data = temp;
}

void lise_darray_free(lise_darray* darray)
{
	free(darray->data);

	darray->data = NULL;
	darray->size = darray->capacity = darray->stride = 0;
}
