#pragma once

#include "afx.hpp"

namespace cfg {

	// speed of animation
	double time_scale = 5;

	// width and height of grid
	constexpr unsigned int width = 16, height = 9;

	// scale of each tile
	double display_scale = 3;

	// size of a sprite square
	vec2u sprite_size = {16, 16};

	// name of the game
	constexpr const char* name = "Basalt Band";

	// debug overlays
	bool weight_overlay = false;
	bool danger_overlay = false;

} // namespace cfg

