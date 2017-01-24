/* -*- cpp.doxygen -*- */
#pragma once

#include <utility>
#include <sfml/window.hpp>
#include <sfml/graphics/renderwindow.hpp>

#include "include/vector.hpp"

#include "config.hpp"

/**
 * \class stdwindow
 * \brief sfml window singleton
 *
 * Since most programs only need one window, this class is provided to provide
 * a way to access and control it. Generally, it acts as a `pointer', using the
 * dereference operators to access the underlying singleton.
 *
 * The configuration of the window is done using macros. See config.hpp for
 * defaults, as well as the format of certain options.
 */
class stdwindow
{
public: // statics

	using window_type = sf::RenderWindow;

	// confix
	static constexpr auto winname = CFG_WINNAME;
	static constexpr auto winstyle = CFG_WINSTYLE;
	static constexpr auto winfps = CFG_WINFPS;
	static const vec2i winsize;

private: // internal statics

	static window_type& get_win()
	{
		static window_type win;
		return win;
	}

public: // methods

	// base accessors
	// go through here, so he are not as affected if we change the window handling

	window_type& window()
	{
		return get_win();
	}

	const window_type& window() const
	{
		return get_win();
	}

	// helpers

	void init()
	{
		this->window().create(sf::VideoMode(winsize.x, winsize.y), winname, winstyle);
		this->window().setFramerateLimit(winfps);
	}

	explicit operator bool() const
	{
		return this->window().isOpen();
	}

	bool operator!() const
	{
		return !bool(*this);
	}

	// direct access

	window_type* operator->()
	{
		return &this->window();
	}

	window_type& operator*()
	{
		return this->window();
	}

	operator window_type&()
	{
		return this->window();
	}

	const window_type* operator->() const
	{
		return &this->window();
	}

	const window_type& operator*() const
	{
		return this->window();
	}

	operator const window_type&() const
	{
		return this->window();
	}

};

const vec2i stdwindow::winsize(CFG_WINSIZE);

stdwindow stdwin;
