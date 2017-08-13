#pragma once

#include "afx.hpp"

// data for single tile is here
// nothing here indicates the position
struct grid_tile
{ // {{{
	button tile;
	bool blocked = false;
	entityx::Entity piece;

public: // methods

	void init(vec2 origin, vec2 size)
	{
		tile = button(origin, size);
	}

	auto handle_event(const sf::Event& event)
	{
		return tile.process(event);
	}

	// uses button's dimensions - use init first
	void draw(sf::Color color)
	{
		if(blocked) {
			return;
		}

		auto region = tile.region();
		sf::RectangleShape rect;
		rect.setSize(vec2{region.width, region.height});
		rect.setPosition(region.left, region.top);
		rect.setFillColor(color);

		stdwin->draw(rect);
	}
}; // }}}

