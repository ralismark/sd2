/* -*- cpp.doxygen -*- */
#pragma once

#include <utility>
#include <sfml/window.hpp>
#include <sfml/graphics/renderwindow.hpp>

#include "include/vector.hpp"

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
	static constexpr auto winname = "Standard window";
	static int winstyle;
	static int winfps;
	static vec2i winsize;

private: // internal statics

	static window_type& get_win();

public: // methods

	// base accessors
	// go through here, so he are not as affected if we change the window handling

	window_type& window();
	const window_type& window() const;

	// helpers

	void init();

	explicit operator bool() const;
	bool operator!() const;

	// direct access

	window_type* operator->();
	window_type& operator*();
	operator window_type&();

	const window_type* operator->() const;
	const window_type& operator*() const;
	operator const window_type&() const;

};

extern stdwindow stdwin;
