/* -*- cpp.doxygen -*- */
#pragma once

#ifndef NDEBUG
#define NDEBUG 1
#endif

#include <iostream>
#include <typeinfo>

#include "disp/window.cpp"
#include "core/event.hpp"
#include "include/sigslots.hpp"
#include "core/opts.cpp"

/**
 * \namespace rt
 * \brief program runtime helpers
 *
 * rt acts as a framework for programs by providing generic functionality
 * required in most programs.
 */
namespace rt {

	/**
	 * \namespace rt::detail
	 * \brief rt internals. Don't use directly.
	 */
	namespace detail {

		/**
		 * \struct exit_signaller
		 * \brief message to exit the program
		 *
		 * Do not use this class explicitly; instead use exit() to
		 * cause an exit.
		 */
		struct exit_signaller
		{
			int exit_code; ///< passed exit code
		};

		struct skipframe_signaller
		{
		};

	} // namespace detail

	/**
	 * \fn exit
	 * \brief causes the program to exit
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
		noexcept(false)
	{
		throw detail::exit_signaller{code};
	}

	/**
	 * \fn skipframe
	 * \brief skips the current frame to the next one
	 *
	 * As with exit(), an exception is used to jump out of the stack and to
	 * the next frame. This skips incrementing the frame counter.
	 */
	void skipframe()
		noexcept(false)
	{
		throw detail::skipframe_signaller{};
	}

	/**
	 * \var on_cleanup
	 * \brief program cleanup hook
	 *
	 * This is a hook which is executed when the program
	 * is about to exit. Exit signals (from rt::exit) are not permitted
	 * here.
	 */
	signal<> on_cleanup;

	/**
	 * \var on_win_event
	 * \brief window event hook
	 *
	 * This allows the user to respond to window events e.g. mouse clicks.
	 * These are triggered for each event, before the main frame hooks.
	 * Multiple may be triggered per frame.
	 *
	 * Note that the window close event has an extra special handler which
	 * is always triggered after all user-defined events.
	 */
	signal<const sf::Event&> on_win_event;

	/**
	 * \var on_frame
	 * \brief per-frame hook
	 *
	 * This can be used to run things that need to happen each frame.
	 * Priorities should be used when adding slots to enforce the execution
	 * order.
	 */
	signal<> on_frame;

	/**
	 * \var frame
	 * \brief frame counter
	 *
	 * This indicated the number of frames that have elapsed since the
	 * beginning of the program.
	 */
	unsigned long long frame;

	/**
	 * \var argc
	 * \var argv
	 * \var pgname
	 * \brief program parameters
	 *
	 * These are set to be the argc and argv passed to main as command line
	 * arguments. These may be modified for internal use before being
	 * available to the developer. Note that GUI programs are usually not
	 * provided with arguments.
	 *
	 * pgname is set to argv[0] - the name of the program.
	 */
	int argc;
	char** argv;
	char* pgname;

} // namespace rt

/**
 * \fn initial
 * \brief user-defined program initialization function
 *
 * This provides a hook into main to initialize the program state. This
 * function must be defined, even if empty. This can be used
 * to install the required slots.
 */
void initial();

int main(int argc, char** argv) try
{
	rt::argc = argc;
	rt::argv = argv;
	rt::pgname = argv[0];

	auto opt_parse_result = rt::opt::parse(argc, argv);
	if(opt_parse_result == rt::opt::parse_help) {
		return 0;
	} else if(opt_parse_result == rt::opt::parse_fail) {
		return 1;
	}

	stdwindow::winstyle = rt::opt::wstyle;
	stdwindow::winfps = rt::opt::wfps.value_or(0);
	stdwindow::winsize = rt::opt::wsize;

	int exit_code = 0;
	try {
		initial();
		if(!stdwin) {
			stdwin.init();
		}

		while(stdwin) {
			try {
				// event loop, see event.hpp for details on event_queue
				for(auto&& event : event_queue(stdwin)) {
					rt::on_win_event(event);

					// always close and exit
					if(event.type == sf::Event::Closed) {
						if(stdwin) {
							stdwin->close();
						}
						rt::exit(0);
					}
				}
				rt::on_frame();

				++rt::frame;
			} catch(const rt::detail::skipframe_signaller& e) {
				(void)(e);
				// go to next frame
			}
		}
	} catch(const rt::detail::exit_signaller& e) {
		// storing the exit code allows cleanup even
		// when exiting
		exit_code = e.exit_code;
	}

	try {
		rt::on_cleanup();
	} catch(const rt::detail::exit_signaller& e) {
		exit_code = e.exit_code;
	}

	return exit_code;
} catch(const std::exception& e) {
	std::cerr << "\nuncaught exception: " << typeid(e).name() << '\n'
		<< "    .what(): " << e.what() << '\n';
} catch(...) {
	std::cerr << "\nuncaught exception!\n";
}
