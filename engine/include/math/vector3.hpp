#pragma once

#include "math/arithmetic.hpp"
#include "math/math.hpp"
#include "definitions.hpp"

namespace lise
{

template<arithmetic A>
struct vector3
{
	union { A x, r, u; };
	union { A y, g, v; };
	union { A z, b, w; };

	A length_squared() const
	{
		return x * x + y * y + z * z;
	}

	A length() const
	{
		return sqrt(length_squared());
	}

	void normalize()
	{
		A len = length();

		x /= len;
		y /= len;
		y /= len;
	}

	vector3<A> normalized() const
	{
		vector3<A> out = *this;

		out.normalize();

		return out;
	}

	A dot(const vector3<A>& r) const
	{
		A p = 0;

		p += x * r.x;
		p += y * r.y;
		p += z * r.z;

		return p;
	}

	vector3<A> cross(const vector3<A>& r) const
	{
		return vector3<A>
		{
			y * r.z - z * r.y,
			z * r.x - x * r.z,
			x * r.y - y * r.x
		};
	}
};

template <arithmetic A>
bool operator == (const vector3<A>& l, const vector3<A>& r)
{
    return l.x == r.x && l.y == r.y && l.z == r.z;
}

template<arithmetic A>
vector3<A> operator - (const vector3<A>& l, const vector3<A>& r)
{
	return vector3<A> {
		l.x - r.x,
		l.y - r.y,
		l.z - r.z
	};
}

typedef vector3<int> vector3i;
typedef vector3<float> vector3f;
typedef vector3<double> vector3d;

//struct vector3i : public vector3<int> {};
//
//struct vector3f : public vector3<float> {};
//
//struct vector3d : public vector3<double> {};

}
