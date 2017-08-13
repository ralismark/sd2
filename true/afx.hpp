#pragma once

#include "core/runtime.cpp"
#include "include/vector.hpp"
#include "include/randutils.hpp"
#include "include/entityx.hpp"
#include "include/fmt.hpp"
#include "res0.hpp"

#include "input/button.hpp"
#include "input/keystate.hpp"
#include "core/math_constants.hpp"
#include "core/delay.hpp"

#include <map>
#include <utility>
#include <vector>
#include <string>
#include <bitset>
#include <algorithm>
#include <iostream>
#include <array>
#include <cmath>
#include <memory>
#include <sfml/graphics.hpp>
#include <sfml/audio.hpp>

// check if a value can be found in range
template <typename It, typename T>
inline bool contains(It first, It last, const T& val)
{
	return std::find(first, last, val) != last;
}

using namespace std::literals;

extern randutils::default_rng rng;

namespace var {

	extern int score;
	extern int score_bonus;
	extern sf::Music bg;

} // namespace var

#include "config.cpp"
