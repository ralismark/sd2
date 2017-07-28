/* -*- cpp.doxygen -*- */
#pragma once

#include <string>

#include "include/optional.hpp"
#include "include/vector.hpp"

namespace rt {

	/**
	 * \namespace opt
	 * \brief non-generic argument parser
	 *
	 * The argument parser here is not generic, only parsing values for the
	 * variables below. This should not be used directly, but instead
	 * through runtime.hpp.
	 */

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

		/**
		 * \fn next_opt
		 * \brief get index of next argument
		 *
		 * This returns optind, which is the index of the next
		 * non-option argument.
		 */
		int next_arg();

	} // namespace opt

} // namespace rt
