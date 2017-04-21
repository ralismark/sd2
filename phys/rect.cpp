#include "rect.hpp"

rect::rect(vec2 hsize_init)
	: hsize(hsize_init), centre()
{
}

rect::rect(vec2 centre_init, vec2 hsize_init)
	: hsize(hsize_init), centre(centre_init)
{
}

vec2 rect::min() const
{
}

rect rect::offset(vec2 off)
{
	return rect{hsize, centre + off};
}

vec2 rect::mtv_from(const rect& other) const
{
	auto get_sign = [] (const vec2& v) {
		return vec2{v.x >= 0 ? 1 : -1,
			    v.y >= 0 ? 1 : -1};
	};

	vec2 hsize_sum = abs(hsize) + abs(other.hsize);
	vec2 diff = centre - other.centre;

	if(any(lessThan(hsize_sum, abs(diff)))) {
		return vec2{0, 0}; // not intersecting
	}

	vec2 diff_trans = get_sign(diff) * hsize_sum - diff;
	if(std::abs(diff_trans.x) < std::abs(diff_trans.y)) {
		return vec2{diff_trans.x, 0};
	} else {
		return vec2{0, diff_trans.y};
	}
}

bool rect::intersects(const rect& other) const
{
	vec2 hsize_sum = abs(hsize) + abs(other.hsize);
	vec2 diff = centre - other.centre;

	// true == no axes have separations
	return !any(lessThan(hsize_sum, abs(diff)));
}

bool rect::contains(const vec2& other) const
{
	// just like intersecting with a 0-size rect
	vec2 diff = centre - other;
	return !any(lessThan(hsize, abs(diff)));
}
