/* -*- cpp.doxygen -*- */
#pragma once

#include <string>
#include <getopt.h>
#include <sfml/window/windowstyle.hpp>

#include "include/fmt.hpp"
#include "include/optional.hpp"
#include "include/vector.hpp"

namespace rt {

	namespace opt {

		/**
		 * \var a
		 * \var b
		 * \var c
		 * \brief programmer-defined options
		 *
		 * These options allow the programmer to handle extra options
		 * specific to their program. Their use depends on the program.
		 */
		stx::optional<std::string> a;
		stx::optional<std::string> b;
		stx::optional<std::string> c;

		stx::optional<int> wfps = 30;
		int wstyle = sf::Style::Titlebar | sf::Style::Close;
		vec2i wsize{1024, 600};

		enum opt_result
		{
			parse_success,
			parse_help,
			parse_fail
		};

		/**
		 * \fn parse
		 * \brief parse arguments
		 *
		 * This is a large function wrapping getopt, and handles the
		 * parsing and setting of all available options.
		 */
		opt_result parse(int argc, char** argv)
		{
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
			auto parse_int = [pgname = argv[0], err] (const std::string& s, const std::string& disp_s) -> stx::optional<int>
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
					return stx::nullopt;
				};

			static option long_opts[] = {
				{"fps",   optional_argument, 0, 'f'},
				{"style", optional_argument, 0, 'w'},
				{"size",  required_argument, 0, 's'},
				{"help",  no_argument,       0, 'h'},
			};

			int opt_index;
			int opt;
			while((opt = getopt_long(argc, argv, "hf::w::s:a::b::c::", long_opts, &opt_index)) != -1) {
				std::string arg = optarg ? optarg : "";

				switch(opt) {
				case 'a':
					a = optarg ? optarg : "";
					break;
				case 'b':
					b = optarg ? optarg : "";
					break;
				case 'c':
					c = optarg ? optarg : "";
					break;
				case 'h':
					fmt::print(help, argv[0]);
					return parse_help;
				case 'f':
					if(optarg) {
						wfps = parse_int(arg, arg);
						if(!wfps) {
							return parse_fail;
						}
					} else {
						wfps = stx::nullopt;
					}
					break;
				case 'w':
					wstyle = 0;
					if(optarg) {
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
								fmt::print(err, argv[0], arg, "invalid character flag");
								return parse_fail;
							}
						}
						if((wstyle & sf::Style::Fullscreen) && (wstyle != sf::Style::Fullscreen)) {
							fmt::print(err, argv[0], arg, "fullscreen cannot be combined with other flags");
							return parse_fail;
						}
					}
					break;
				case 's':
					{
						auto split = arg.find_first_of(',');
						if(split == std::string::npos) {
							fmt::print(err, argv[0], arg, "invalid format");
							return parse_fail;
						}
						auto x = parse_int(arg.substr(0, split), arg);
						if(!x) {
							return parse_fail;
						}
						auto y = parse_int(arg.substr(split + 1), arg);
						if(!y) {
							return parse_fail;
						}

						wsize.x = *x;
						wsize.y = *y;
					}
					break;
				case '?':
				case ':':
					return parse_fail;
				default:
					throw std::logic_error("unhandled option");
				}
			}
			return parse_success;
		}

	} // namespace opt

} // namespace rt
