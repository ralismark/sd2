#include "event.hpp"

// class event_iterator {{{

event_iterator::event_iterator()
	: owner(nullptr)
{
}

event_iterator::event_iterator(stdwindow& init)
	: owner(&init)
{
	++*this; // get first element
}

const event_iterator::value_type& event_iterator::operator*() const
{
	return event;
}

event_iterator& event_iterator::operator++()
{
	if(!owner->window().pollEvent(event)) {
		owner = nullptr;
	}
	return *this;
}

event_iterator event_iterator::operator++(int)
{
	auto cpy = *this;
	++(*this);
	return cpy;
}

bool event_iterator::operator==(const event_iterator& other) const
{
	return owner == other.owner;
}

bool event_iterator::operator!=(const event_iterator& other) const
{
	return !(*this == other);
}

// }}}

// class event_queue {{{

event_queue::event_queue(stdwindow& init)
	: owner(&init)
{
}

event_iterator event_queue::begin()
{
	return { *owner };
}

event_iterator event_queue::end()
{
	return {};
}

// }}}
