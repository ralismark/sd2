#include "rotation.hpp"

#include "core/math_constants.hpp"

#include <cmath>

static double wrap_into_range(double val)
{
	return std::fmod(val, constant::pi());
}

// struct angle {{{

angle angle::from_radians(double rad)
{
	return angle{wrap_into_range(rad)};
}

angle angle::from_degrees(double deg)
{
	return angle{wrap_into_range(deg * constant::pi() / 180)};
}

angle angle::atan2(const vec2& direction)
{
	return angle{std::atan2(direction.y, direction.x)};
}

angle::angle(double rad)
	: rad(rad)
{
}

angle::angle()
	: rad(0)
{
}

// getters {{{

double angle::radians() const
{
	return rad;
}

double angle::degrees() const
{
	return rad * 180 / constant::pi();
}

double angle::sin() const
{
	return std::sin(rad);
}

double angle::cos() const
{
	return std::cos(rad);
}

double angle::tan() const
{
	return std::tan(rad);
}

// }}}
// unary {{{

angle operator+(const angle& val)
{
	return val;
}

angle operator-(const angle& val)
{
	return angle{-val.rad};
}

// }}}
// add/sub {{{

angle operator+(angle lhs, const angle& rhs)
{
	return lhs += rhs;
}

angle operator-(angle lhs, const angle& rhs)
{
	return lhs -= rhs;
}

angle& operator+=(angle& lhs, const angle& rhs)
{
	lhs.rad = wrap_into_range(lhs.rad + rhs.rad);
	return lhs;
}

angle& operator-=(angle& lhs, const angle& rhs)
{
	lhs.rad = wrap_into_range(lhs.rad - rhs.rad);
	return lhs;
}

// }}}
// mul/div {{{

angle operator*(angle lhs, double rhs)
{
	return lhs *= rhs;
}

angle operator*(double lhs, angle rhs)
{
	return rhs *= lhs;
}

angle& operator*=(angle& lhs, double rhs)
{
	lhs.rad = wrap_into_range(lhs.rad * rhs);
	return lhs;
}

angle operator/(angle lhs, double rhs)
{
	return lhs /= rhs;
}

angle& operator/=(angle& lhs, double rhs)
{
	lhs.rad = wrap_into_range(lhs.rad / rhs);
	return lhs;
}

// }}}
// relational {{{

bool operator==(const angle& lhs, const angle& rhs)
{
	return lhs.rad == rhs.rad;
}

bool operator!=(const angle& lhs, const angle& rhs)
{
	return lhs.rad != rhs.rad;
}

bool operator<(const angle& lhs, const angle& rhs)
{
	return lhs.rad < rhs.rad;
}

bool operator>(const angle& lhs, const angle& rhs)
{
	return lhs.rad > rhs.rad;
}

bool operator<=(const angle& lhs, const angle& rhs)
{
	return lhs.rad <= rhs.rad;
}

bool operator>=(const angle& lhs, const angle& rhs)
{
	return lhs.rad >= rhs.rad;
}

// }}}
// literals {{{

angle operator ""_deg(long double val)
{
	return angle::from_degrees(val);
}

angle operator ""_rad(long double val)
{
	return angle::from_radians(val);
}

// }}}

// }}}
