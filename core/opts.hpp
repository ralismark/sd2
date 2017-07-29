/* -*- cpp.doxygen -*- */
#pragma once

#include <string>

#include "include/optional.hpp"
#include "include/vector.hpp"

/**
 * \internal
 * \file
 * \brief Argument parser for \c rt
 *
 * This file has the interface for the command line argument parser used in the
 * predefined main(). This serves quite a specific purpose, and is probably not
 * able to be used much elsewhere.
 */

namespace rt {

	/**
	 * \internal
	 * \namespace opt
	 * \brief Non-generic argument parser
	 *
	 * The argument parser here is not generic, only parsing values for the
	 * variables below. This should not be used directly, but instead
	 * through runtime.hpp.
	 */

	namespace opt {

		/**
		 * \internal
		 * \var a
		 * \var b
		 * \var c
		 * \brief Programmer-defined options
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
		 * \internal
		 * \fn parse
		 * \brief Parse arguments
		 *
		 * This is a large function wrapping getopt, and handles the
		 * parsing and setting of all available options.
		 */
		opt_result parse(int argc, char** argv);

		/**
		 * \internal
		 * \fn next_opt
		 * \brief Get index of next argument
		 *
		 * This returns optind, which is the index of the next
		 * non-option argument.
		 */
		int next_arg();

	} // namespace opt

} // namespace rt
