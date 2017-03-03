#include "window.hpp"

// class stdwindow {{{

int stdwindow::winstyle{};
int stdwindow::winfps{};
vec2i stdwindow::winsize{};

stdwindow::window_type& stdwindow::get_win()
{
	static window_type win;
	return win;
}

// base accessors

stdwindow::window_type& stdwindow::window()
{
	return get_win();
}

const stdwindow::window_type& stdwindow::window() const
{
	return get_win();
}

// helpers

void stdwindow::init()
{
	this->window().create(sf::VideoMode(winsize.x, winsize.y), winname, winstyle);
	if(winfps > 0) {
		this->window().setFramerateLimit(winfps);
	}
}

explicit stdwindow::operator bool() const
{
	return this->window().isOpen();
}

bool stdwindow::operator!() const
{
	return !bool(*this);
}

// direct access

stdwindow::window_type* stdwindow::operator->()
{
	return &this->window();
}

stdwindow::window_type& stdwindow::operator*()
{
	return this->window();
}

stdwindow::operator window_type&()
{
	return this->window();
}

const stdwindow::window_type* stdwindow::operator->() const
{
	return &this->window();
}

const stdwindow::window_type& stdwindow::operator*() const
{
	return this->window();
}

stdwindow::operator const window_type&() const
{
	return this->window();
}

// }}}

stdwindow stdwin{};
