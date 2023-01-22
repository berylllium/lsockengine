#pragma once

#include <type_traits>
#include <concepts>

#include "definitions.hpp"

namespace lise
{

template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template<arithmetic A>
struct vector2
{
	A x, y; 

	friend bool operator == (const vector2<A>& l, const vector2<A>& r)
	{
		return l.x == r.x && l.y == r.y;
	}
};

struct vector2i : public vector2<int> {};

struct vector2f : public vector2<float> {};

struct vector2d : public vector2<double> {};

}