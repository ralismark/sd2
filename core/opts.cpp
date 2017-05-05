#include "opts.hpp"

#include <cassert>
#include <getopt.h>
#include <sfml/window/windowstyle.hpp>

#include "include/fmt.hpp"

namespace rt {

	namespace opt {

		stx::optional<std::string> a;
		stx::optional<std::string> b;
		stx::optional<std::string> c;

		stx::optional<int> wfps = 30;
		int wstyle = sf::Style::Titlebar | sf::Style::Close;
		vec2i wsize{1024, 600};

		namespace { // anonymous

			const char* help =
R"({0} - standard window

Usage: {0} [options...]
    -a[=VALUE]          Program defined
    -b[=VALUE]          Program defined
    -c[=VALUE]          Program defined
    -f, --fps[=LIMIT]   Specify the framerate limit, or have no limit
    -w, --style[=STYLE] Specify the window style. A conbination of 't'
                        (titlebar), 'c' (close), 'r' (resize), and
                        'f' (fullscreen, not permitted with others).
    -s, --size=X,Y      Set the width (X) and height (Y) of the window.
    -h, --help          Display this message and exit.

The program defied option are parsed, but their behaviour depends on the
program using this runtime, or may be ignored.
)";

			const char* err = "{}: {}: {}\n";

			stx::optional<int> parse_int(const char* pgname, const std::string& s, const std::string& disp_s)
			{
				try {
					size_t idx = 0;
					int out = std::stoi(s, &idx);
					if(idx < s.size() || out < 1) {
						throw std::invalid_argument("");
					}
					return out;
				} catch(const std::invalid_argument&) {
					fmt::print(err, pgname, disp_s, "invalid format");
				} catch(const std::out_of_range&) {
					fmt::print(err, pgname, disp_s, "too large to be representable");
				}
				// if exception is thrown
				return stx::nullopt;
			}

			bool parse_set_winstyle(const char* pgname, const std::string& arg)
			{
				wstyle = 0;
				for(auto& c : arg) {
					switch(c) {
					case 't':
						wstyle |= sf::Style::Titlebar;
						break;
					case 'r':
						wstyle |= sf::Style::Resize;
						break;
					case 'c':
						wstyle |= sf::Style::Close;
						break;
					case 'f':
						wstyle |= sf::Style::Fullscreen;
						break;
					default:
						fmt::print(err, pgname, arg, "invalid character flag");
						return false;
					}
				}
				if((wstyle & sf::Style::Fullscreen) && (wstyle != sf::Style::Fullscreen)) {
					fmt::print(err, pgname, arg, "fullscreen cannot be combined with other flags");
					return false;
				}
				return true;
			}

			bool parse_set_winsize(const char* pgname, const std::string& arg)
			{
				auto split = arg.find_first_of(',');
				if(split == std::string::npos) {
					fmt::print(err, pgname, arg, "invalid format");
					return false;
				}
				auto x = parse_int(pgname, arg.substr(0, split), arg);
				if(!x) {
					return false;
				}
				auto y = parse_int(pgname, arg.substr(split + 1), arg);
				if(!y) {
					return false;
				}

				wsize.x = *x;
				wsize.y = *y;

				return true;
			}

		} // namespace anonymous

		opt_result parse(int argc, char** argv)
		{
			static option long_opts[] = {
				{"fps",   optional_argument, 0, 'f'},
				{"style", optional_argument, 0, 'w'},
				{"size",  required_argument, 0, 's'},
				{"help",  no_argument,       0, 'h'},
			};

			const char* short_opts = "hf::w::s:a::b::c::";

			int opt_index;
			int opt;
			while((opt = getopt_long(argc, argv, short_opts, long_opts, &opt_index)) != -1) {
				std::string arg = optarg ? optarg : ""; // for convenience

				switch(opt) {
				case 'a':
					a = arg;
					break;
				case 'b':
					b = arg;
					break;
				case 'c':
					c = arg;
					break;
				case 'h':
					fmt::print(help, argv[0]);
					return parse_help;
				case 'f':
					if(optarg) {
						wfps = parse_int(argv[0], arg, arg);
						if(!wfps) {
							return parse_fail;
						}
					} else {
						wfps = stx::nullopt;
					}
					break;
				case 'w':
					if(!parse_set_winstyle(argv[0], arg)) {
						return parse_fail;
					}
					break;
				case 's':
					if(!parse_set_winsize(argv[0], arg)) {
						return parse_fail;
					}
					break;
				case '?':
				case ':':
					return parse_fail;
				default:
					assert(false && "unhandled option");
				}
			}
			return parse_success;
		}

		int next_arg()
		{
			return optind;
		}

	} // namespace opt

} // namespace rt

