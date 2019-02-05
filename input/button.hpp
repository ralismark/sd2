/* -*- cpp.doxygen -*- */
#pragma once

#include <map>
#include <list>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Window/Event.hpp>

#include "include/vector.hpp"

/**
 * \class button
 * \brief Bare button config
 *
 * This class contains a minimal set of members which are able to define a
 * button. This does not contain any functionality - that is provided by the
 * switchboard class.
 *
 * Each button is implemented as a finite state machine, the details can be
 * seen below.
 */
class button
{
public: // statics

	using dimension_type = int;
	using vector_type = vector<int, 2>;

	/*
	 * The state of a button and the cursor can be represented as a state
	 * machine, with 4 distinct states:
	 */
	enum class state {
		idle,    // not pressed, cursor away
		hover,   // not pressed, cursor over
		active,   // is  pressed, cursor over
		persist, // is  pressed, cursor away
	};
	/*
	 * only one status can change in a state transition - either the
	 * proximity (away or over) and the press (or not) of the button.
	 * Between these states, there are 8 possible transitions
	 */
	enum class event {
		hover_on, // idle -> hover
		hover_off, // hover -> idle
		press, // hover -> active
		release, // active -> hover
		leave, // active -> persist
		reenter, // persist -> active
		away_release, // persist -> idle
		// cannot happen: away_press, // idle -> persist
	};

	/*
	 * Since the two variables (press and proximity) are controlled by separate events:
	 * MouseMoved for proximity and MouseButtonPressed and
	 * MouseButtonReleased, there is no way for ambiguity between
	 * transitions, e.g. the mouse both moves and presses the button.
	 */

private: // variables

	sf::Rect<dimension_type> bound;
	state condition;

public: // methods

	button();
	button(const vector_type& pos, const vector_type& size);
	button(const sf::Rect<dimension_type>& init_bound);

	bool contains(const vector_type& pos) const;

	state current_state() const;
	sf::Rect<dimension_type> region() const;
	// Note: This automatically sets the state to idle
	std::list<event> region(sf::Rect<dimension_type> new_bound);

	std::list<event> transition(state new_state);
	std::list<event> process(const sf::Event& e);
};
