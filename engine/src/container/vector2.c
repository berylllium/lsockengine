#include "container/vector2.h"

bool lise_vector2i_equals(lise_vector2i l, lise_vector2i r)
{
	return l.x == r.x && l.y == r.y;
}

bool lise_vector2f_equals(lise_vector2f l, lise_vector2f r)
{
	return l.x == r.x && l.y == r.y;
}

bool lise_vector2d_equals(lise_vector2d l, lise_vector2d r)
{
	return l.x == r.x && l.y == r.y;
}
