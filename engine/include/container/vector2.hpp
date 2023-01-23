#pragma once

#include <type_traits>
#include <concepts>

#include "definitions.hpp"

namespace lise
{

template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;

/**
 * @brief The base two-dimensional vector struct template. This struct template is used to create
 * structs like \ref vector2i and \ref vector2f.
 * 
 * @tparam A The type of the elements of the class.
 */
template<arithmetic A>
struct vector2
{
	A x, y; 

	friend bool operator == (const vector2<A>& l, const vector2<A>& r)
	{
		return l.x == r.x && l.y == r.y;
	}
};

/**
 * @brief A two-dimensional vector of integers.
 * 
 */
struct vector2i : public vector2<int> {};

/**
 * @brief A two-dimensional vector of floats.
 * 
 */
struct vector2f : public vector2<float> {};

/**
 * @brief A two-dimensional vector of doubles.
 * 
 */
struct vector2d : public vector2<double> {};

}