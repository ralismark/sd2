#include "math_constants.hpp"

#include <cmath>

namespace constant {

	long double pi()
	{
		static long double cache = std::acos(-1.0l);
		return cache;
	}

	long double e()
	{
		static long double cache = std::exp(1.0l);
		return cache;
	}

	long double sqrt2()
	{
		static long double cache = std::sqrt(2.0l);
		return cache;
	}

} // namespace constant
