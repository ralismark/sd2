/* -*- cpp.doxygen -*- */
#pragma once

#ifndef NDEBUG
#define NDEBUG 1
#endif

#include <vector>
#include <sfml/window/event.hpp>

#include "include/sigslots.hpp"

/**
 * \file
 * \brief Rhyolite runtime
 *
 * This file has some useful utilities for the basis of a GUI program. Most
 * notably, it has a predefine main(), which performs most setup and per-frame
 * tasks. The user should attach callbacks to rt::on_frame, rt::on_win_event
 * and rt::on_cleanup to perform specific tasks. This, as well as any other
 * initialisation, can be done in initial().
 */

/**
 * \namespace rt
 * \brief Program runtime helpers
 *
 * rt acts as a framework for programs by providing generic functionality
 * required in most programs.
 */
namespace rt {

	/**
	 * \internal
	 * \namespace rt::detail
	 * \brief \c rt internals. Don't use directly.
	 */
	namespace detail {

		/**
		 * \internal
		 * \struct exit_signaller
		 * \brief Message to exit the program
		 *
		 * Do not use this class explicitly; instead use exit() to
		 * cause an exit. Alternatively, this can be used as an
		 * exception to unwind the stack (e.g. for exiting a thread).
		 */
		struct exit_signaller
		{
			int exit_code; ///< passed exit code
		};

		/**
		 * \internal
		 * \struct skipframe_signaller
		 * \brief Message to skip to next frame
		 *
		 * Similar to exit_signaller, do not throw this explicitly, but
		 * use skipframe() instead.
		 */
		struct skipframe_signaller
		{
		};

	} // namespace detail

	/**
	 * \fn exit
	 * \brief Causes the program to exit
	 *
	 * This uses an exception to safely exit the program. Unlike the C exit
	 * functions (exit, quick_exit), this will unwind the stack.
	 *
	 * One caveat to this is that a catch-all handler may stop the exit
	 * signal.
	 */
	void exit(
	          int code = 0 ///< [in] intended exit code, defaulting to 0
	         )
		noexcept(false);

	/**
	 * \fn skipframe
	 * \brief Skips the current frame to the next one
	 *
	 * As with exit(), an exception is used to jump out of the stack and to
	 * the next frame. This skips incrementing the frame counter.
	 */
	void skipframe()
		noexcept(false);

	/**
	 * \var on_cleanup
	 * \brief Program cleanup hook
	 *
	 * This is a hook which is executed when the program
	 * is about to exit. Exit signals (from rt::exit) are not permitted
	 * here.
	 */
	extern signal<> on_cleanup;

	/**
	 * \var on_win_event
	 * \brief Window event hook
	 *
	 * This allows the user to respond to window events e.g. mouse clicks.
	 * These are triggered for each event, before the main frame hooks.
	 * Multiple may be triggered per frame.
	 *
	 * Note that the window close event has an extra special handler which
	 * is always triggered after all user-defined events.
	 */
	extern signal<const sf::Event&> on_win_event;

	/**
	 * \var on_frame
	 * \brief Per-frame hook
	 *
	 * This can be used to run things that need to happen each frame.
	 * Priorities should be used when adding slots to enforce the execution
	 * order.
	 */
	extern signal<> on_frame;

	/**
	 * \var frame
	 * \brief Frame counter
	 *
	 * This indicated the number of frames that have elapsed since the
	 * beginning of the program.
	 */
	extern unsigned long long frame;

	/**
	 * \var argc
	 * \var argv
	 * \var pgname
	 * \brief Program parameters
	 *
	 * These are set to be the argc and argv passed to main as command line
	 * arguments. These may be modified for internal use before being
	 * available to the developer. Note that GUI programs are usually not
	 * provided with arguments.
	 *
	 * pgname is set to argv[0] - the name of the program.
	 */
	extern int argc;
	extern char** argv;
	extern char* pgname;

	/**
	 * \var args
	 * \brief Arguments after options
	 *
	 * This vector contains the arguments to the program, excluding
	 * options.
	 */
	extern std::vector<char*> args;

} // namespace rt

/**
 * \fn initial
 * \brief User-defined program initialization function
 *
 * This provides a hook into main to initialize the program state. This
 * function must be defined, even if empty. This can be used
 * to install the required slots.
 */
void initial();

/**
 * \fn main(int argc, char** argv)
 * \brief Main program
 *
 * This is defined by the runtime, and sets up several of its components. It
 * also handles the event loop, as well as frame events.
 */
int main(int argc, char** argv);
