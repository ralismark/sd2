#include "runtime.hpp"

#include <iostream>
#include <typeinfo>

#include "core/event.hpp"
#include "core/opts.hpp"
#include "disp/window.hpp"

namespace rt {

	void exit(int code)
		noexcept(false)
	{
		throw detail::exit_signaller{code};
	}

	void skipframe()
		noexcept(false)
	{
		throw detail::skipframe_signaller{};
	}

	signal<> on_cleanup;
	signal<const sf::Event&> on_win_event;
	signal<> on_frame;

	unsigned long long frame;

	int argc;
	char** argv;
	char* pgname;

} // namespace rt

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
