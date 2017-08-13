#pragma once

#include "afx.hpp"

// grid of grid_tiles
struct grid_type
{ // {{{
public: // variables

	vec2 origin, size;
	grid_of<grid_tile> grid; // column major

public: // methods

	void init()
	{
		for(int x = 0; x < cfg::width; ++x) {
			for(int y = 0; y < cfg::height; ++y) {
				grid[x][y].init(origin + size * vec2(x, y), size);
			}
		}
	}

	grid_of<std::list<button::event>> dispatch_event(const sf::Event& event)
	{
		grid_of<std::list<button::event>> output;

		for(int x = 0; x < cfg::width; ++x) {
			for(int y = 0; y < cfg::height; ++y) {
				output[x][y] = grid[x][y].handle_event(event);
			}
		}
		return output;
	}
}; // }}}

namespace var {

	grid_type grid;

} // namespace var

grid_tile& tile(unsigned int x, unsigned int y)
{
	return var::grid.grid[x][y];
}
