#pragma once

#include "definitions.h"

#define LISE_DARRAY_DEFAULT_CAPACITY 1
#define LISE_DARRAY_RESIZE_FACTOR 2

typedef struct lise_darray
{
	uint64_t size;
	uint64_t capacity;
	uint64_t stride;

	void* data;
} lise_darray;

LAPI lise_darray _lise_darray_create(uint64_t default_capacity, uint64_t stride);

LAPI void* lise_darray_get(lise_darray* darray, uint64_t index);

LAPI void lise_darray_push_back(lise_darray* darray, void* element);

LAPI void lise_darray_resize(lise_darray* darray, uint64_t new_capacity);

LAPI void lise_darray_free(lise_darray* darray);

#define lise_darray_create(type) \
	_lise_darray_create(LISE_DARRAY_DEFAULT_CAPACITY, sizeof(type))
