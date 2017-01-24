#pragma once

#include <sfml/window.hpp>
#include "window.cpp"

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
