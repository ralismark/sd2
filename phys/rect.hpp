#pragma once

#include "include/vector.hpp"

struct rect
{
public: // variables

	vec2 centre;
	vec2 hsize;

public: // methods

	rect(vec2 hsize_init);
	rect(vec2 centre_init, vec2 hsize_init);

	vec2 min() const;
	vec2 max() const;
	vec2 size() const;

	rect offset(vec2 off);
	vec2 mtv_from(const rect& other) const;

	bool intersects(const rect& other) const;
	bool contains(const vec2& other) const;
};
