#pragma once

#include "math/vector2.hpp"
#include "math/vector3.hpp"

namespace lise
{

struct vertex
{
	vector3f position;
	vector2f tex_coord;
	vector3f normal;
};

}
