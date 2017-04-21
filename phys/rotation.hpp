#pragma once

#include "include/vector.hpp"

/**
 * \struct angle
 * \brief angle wrapper
 *
 * This stores a double, which represents the angle in radians. It also wraps
 * the value, keeping it in the rangle [-pi, pi].
 */
struct angle
{
public: // statics

	static angle from_radians(double rad);
	static angle from_degrees(double deg);
	static angle atan2(const vec2& direction);

private: // variables

	double rad;

private: // internal methods

	explicit angle(double rad);

public: // methods

	angle();

	double radians() const;
	double degrees() const;

	double sin() const;
	double cos() const;
	double tan() const;

	friend angle operator+(const angle& val);
	friend angle operator-(const angle& val);

	friend angle operator+(angle lhs, const angle& rhs);
	friend angle operator-(angle lhs, const angle& rhs);
	friend angle& operator+=(angle& lhs, const angle& rhs);
	friend angle& operator-=(angle& lhs, const angle& rhs);

	friend angle operator*(angle lhs, double rhs);
	friend angle operator*(double rhs, angle rhs);
	friend angle& operator*=(angle& lhs, double rhs);

	friend angle operator/(angle lhs, double rhs);
	friend angle& operator/=(angle& lhs, double rhs);

	friend bool operator==(const angle& lhs, const angle& rhs);
	friend bool operator!=(const angle& lhs, const angle& rhs);
	friend bool operator<(const angle& lhs, const angle& rhs);
	friend bool operator>(const angle& lhs, const angle& rhs);
	friend bool operator<=(const angle& lhs, const angle& rhs);
	friend bool operator>=(const angle& lhs, const angle& rhs);

	friend angle operator ""_deg(long double val);
	friend angle operator ""_rad(long double val);

};
