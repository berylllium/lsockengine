#pragma once

#include "math/arithmetic.hpp"
#include "definitions.hpp"

namespace lise
{

// Pre-define friend-template functions so the compiler knows they're templated.
template <arithmetic A> struct vector4;
template <arithmetic A> bool operator == (const vector4<A>& l, const vector4<A>& r)
{
    return l.x == r.x && l.y == r.y && l.z == r.z && l.w == r.w;
}

template<arithmetic A>
struct vector4
{
    union { A x, r; };
    union { A y, g; };
	union { A z, b; };
	union { A w, a; };

    friend bool operator ==<> (const vector4<A>& l, const vector4<A>& r);
};

struct vector4i : public vector4<int> {};

struct vector4f : public vector4<float> {};

struct vector4d : public vector4<double> {};

}
