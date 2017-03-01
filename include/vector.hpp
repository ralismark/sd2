#pragma once

#include "preproc.hpp"

_pp_warn_push();

#if defined(COMPILER_MSVC)
	_pp_warn_stop(4244);
#endif


#include "depends/CxxSwizzle/include/swizzle/glsl/scalar_support.h"
#include "depends/CxxSwizzle/include/swizzle/glsl/vector.h"

_pp_warn_pop();

template <typename T, size_t N>
using vector = swizzle::glsl::vector<T, N>;

using vec2  = vector<double,       2>;
using vec2i = vector<int,          2>;
using vec2u = vector<unsigned int, 2>;

using vec3  = vector<double,       3>;
using vec3i = vector<int,          3>;
using vec3u = vector<unsigned int, 3>;

