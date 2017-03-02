/* -*- cpp.doxygen -*- */
#pragma once

#include <sfml/window.hpp>
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
private: // variables

	stdwindow* owner;
	sf::Event event;

public: // methods

	event_iterator()
		: owner(nullptr)
	{
	}

	event_iterator(stdwindow& init)
		: owner(&init)
	{
		++*this; // get first element
	}

	const sf::Event& operator*() const&
	{
		return event;
	}

	sf::Event operator*() const&&
	{
		return event;
	}

	event_iterator& operator++()
	{
		if(!owner->window().pollEvent(event)) {
			owner = nullptr;
		}
		return *this;
	}

	event_iterator operator++(int)
	{
		auto cpy = *this;
		++(*this);
		return cpy;
	}

	bool operator==(const event_iterator& other) const
	{
		return owner == other.owner;
	}

	bool operator!=(const event_iterator& other) const
	{
		return !(*this == other);
	}

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

	event_queue(stdwindow& init)
		: owner(&init)
	{
	}

	event_iterator begin()
	{
		return { *owner };
	}

	event_iterator end()
	{
		return {};
	}
};
