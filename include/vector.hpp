#pragma once

#include "../depends/CxxSwizzle/include/swizzle/glsl/scalar_support.h"
#include "../depends/CxxSwizzle/include/swizzle/glsl/vector.h"

template <typename T, size_t N>
using vector = swizzle::glsl::vector<T, N>;
