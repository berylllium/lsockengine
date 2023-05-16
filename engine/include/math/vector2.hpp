#pragma once

#include "arithmetic.hpp"
#include "definitions.hpp"

namespace lise
{

template<arithmetic A>
struct vector2
{
    union { A x, s, u, w; };
    union { A y, t, v, h; };
};

template <arithmetic A>
bool operator == (const vector2<A>& l, const vector2<A>& r)
{
    return l.x == r.x && l.y == r.y;
}


typedef vector2<int> vector2i;
typedef vector2<unsigned int> vector2ui;
typedef vector2<float> vector2f;
typedef vector2<double> vector2d;

//struct vector2i : public vector2<int> {};
//
//struct vector2f : public vector2<float> {};
//
//struct vector2d : public vector2<double> {};

}
