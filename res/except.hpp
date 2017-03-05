/* -*- cpp.doxygen -*- */
#pragma once

#include <stdexcept>

namespace res {

	/**
	 * \class invalid_key
	 * \brief exception for guaranteed invalid key
	 *
	 * This exception should be used when the API user attempts to use an
	 * invalid key (e.g. one not found in a lookup table). However, it
	 * should not be used if the resource may or may not be present (e.g. a
	 * file); in this case, use resource_unavailable.
	 */
	class invalid_key
		: public std::invalid_argument
	{
	public:

		using std::invalid_argument::invalid_argument;

		invalid_key();
		virtual ~invalid_key();

	};

	/**
	 * \class unavailable
	 * \brief exception for temporarily unavailable resource (EAGAIN)
	 *
	 * This should be used when resource loading would normally succeed,
	 * but does not in the current situation (for various reasons). This
	 * class should not be used if the key is never valid; instead use
	 * invalid_key.
	 */
	class unavailable
		: public std::runtime_error
	{
	public:

		using std::runtime_error::runtime_error;

		unavailable();
		virtual ~unavailable();

	};

} // namespace res
