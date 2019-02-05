/* -*- cpp.doxygen -*- */
#pragma once

#include <SFML/Window.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "include/vector.hpp"

/**
 * \file
 * \brief SFML window wrapper
 *
 * This provides a global variable (#stdwin), which can be used to access a
 * singleton window.
 */

/**
 * \class stdwindow
 * \brief SFML window singleton
 *
 * Since most programs only need one window, this class is provided to provide
 * a way to access and control it. Generally, it acts as a `pointer', using the
 * dereference operators to access the underlying singleton.
 *
 * If using the provided main(), the configuration can be modified using
 * command line arguments.
 *
 * \warning
 * The init() method must be called before using the window. If you're using
 * the provided main() in the runtime, this is already done for you.
 */
class stdwindow
{
public: // statics

	/// Underlying singleton window type
	using window_type = sf::RenderWindow;

	/// Window name
	static constexpr auto winname = "Standard window";

	/// Window style, using SFML window styles
	static int winstyle;

	/// FPS Limit. If negative or 0, no limit is set
	static int winfps;

	/// Window drawable area size
	static vec2i winsize;

private: // internal statics

	static window_type& get_win();

public: // methods

	// base accessors
	// go through here, so he are not as affected if we change the window handling

	window_type& window();
	const window_type& window() const;

	// helpers

	/// Initialise the window, using the static variables. Do not initialise more than once
	void init();

	/// Find if the window has been initialised
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

/**
 * \var stdwin
 * \brief Instance of stdwindow
 *
 * This provides a unified access point to the underlying window singleton in
 * stdwindow. Use this if you want, since all instances point to the same
 * window.
 */
extern stdwindow stdwin;
