#pragma once

#include "definitions.h"

typedef struct lise_vector2i
{
	int32_t x, y;	
} lise_vector2i;

typedef struct lise_vector2f
{
	float x, y;
} lise_vector2f;

typedef struct lise_vector2d
{
	double x, y;
} lise_vector2d;

LAPI bool lise_vector2i_equals(lise_vector2i l, lise_vector2i r);

LAPI bool lise_vector2f_equals(lise_vector2f l, lise_vector2f r);

LAPI bool lise_vector2d_equals(lise_vector2d l, lise_vector2d r);
