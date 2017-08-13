#pragma once

#include "afx.hpp"

// interface to get tiles and such without requiring the actual grid

inline unsigned int square_to_index(vec2u square)
{
	return square.y * cfg::width + square.x;
}

inline vec2u index_to_square(unsigned int n)
{
	return { n % cfg::width, n / cfg::width };
}

template <typename T>
using grid_of = std::array<std::array<T, cfg::height>, cfg::width>;

grid_tile& tile(unsigned int x, unsigned int y);

grid_tile& tile(vec2u pos)
{
	return tile(pos.x, pos.y);
}
