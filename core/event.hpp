/* -*- cpp.doxygen -*- */
#pragma once

/**
 * \file
 * \brief SFML Window event loop helpers
 *
 * This file provides the event_queue and event_iterator classes, which provide
 * a simpler interface for the event loop. Additionally, it allows the use of
 * range-for loops to loop over pending events.
 */

#include <sfml/window/event.hpp>
#include "disp/window.hpp"

/**
 * \class event_iterator
 * \brief Input iterator for getting window events
 *
 * This is a way of getting sfml window event through an iterator. Combined
 * with event_queue, this allows a range-for loop to be used, which cleans up
 * code.
 */
class event_iterator
{
public: // statics
	using value_type = const sf::Event&;
private: // variables

	stdwindow* owner;
	sf::Event event;

public: // methods

	event_iterator();
	event_iterator(stdwindow& init);

	const value_type& operator*() const;

	event_iterator& operator++();
	event_iterator operator++(int);

	bool operator==(const event_iterator& other) const;
	bool operator!=(const event_iterator& other) const;

};

/**
 * \class event_queue
 * \brief event_iterator creator
 *
 * This is a container-like class that allows iterating over the pending events
 * of a window. begin() and end() members are provided to create this
 * functionality.
 */
class event_queue
{
private: // variables

	stdwindow* owner;

public: // methods

	event_queue(stdwindow& init);

	event_iterator begin();
	event_iterator end();
};
