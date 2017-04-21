#pragma once

#include "include/vector.hpp"

struct locator
{
public: // variables

	vec2 pos;
	vec2 vel;

public: // methods

	locator(vec2 init_pos, vec2 init_vel = {0, 0})
		: pos(init_pos), vel(init_vel)
	{
	}

	void advance(double dt)
	{
		pos += vel * dt;
	}

	void accelerate(vec2 force, double dt = 0)
	{
		vel += force * dt;
	}
};
