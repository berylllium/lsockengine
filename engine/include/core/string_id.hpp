#pragma once

#include <string>

#include "math/crc.hpp"

namespace lise
{

typedef uint64_t StringId;

constexpr StringId operator ""_sid(const char* arr, std::size_t size) { return hash_crc64(0, arr, size); }

inline StringId to_sid(const std::string& s)
{
	return hash_crc64(0, s.c_str(), s.length());
}

}
