#pragma once

#include "definitions.h"

typedef struct darray
{
	uint64_t capacity;
	uint64_t size;
	uint32_t stride;
	void* data;
} darray;

LAPI darray darray_create(uint64_t size, uint32_t stride);

