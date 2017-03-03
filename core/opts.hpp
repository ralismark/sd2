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
		extern stx::optional<std::string> a;
		extern stx::optional<std::string> b;
		extern stx::optional<std::string> c;

		extern stx::optional<int> wfps;
		extern int wstyle;
		extern vec2i wsize;

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
		opt_result parse(int argc, char** argv);

	} // namespace opt

} // namespace rt
