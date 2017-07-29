#pragma once

#include "core/time.hpp"

/**
 * \file
 * \brief Helper for delayed/continuous actions
 *
 * This file only declares rt::delay_runner, which is a much nicer wrapper
 * around exec_at and exec_until.
 */

namespace rt {

	/**
	 * \struct delay_runner
	 * \brief Helper to do delayed/continuous actions
	 *
	 * Construct this object, then use the member functions to do things
	 * with a delay. This is nicer wrapper around rt::exec_at and
	 * rt::exec_until in time.hpp.
	 *
	 * \bug Currently, issues arise from using another instance of this class
	 * while running a callback.
	 */
	struct delay_runner
	{
	private:
		// used as an additional offset after current_offset, for
		// non-also methods
		clock::duration future_offset;
		// used for the beginning of events with also_*
		clock::time_point current_offset;
	public:
		/// Creates a runner, possibly with an extra offset
		delay_runner(clock::duration offset = clock::duration::zero());

		/// Returns the current offset time for events
		clock::time_point when() const;

		/// Delays the current offset by \p d, and resetting the future offset
		delay_runner& delay(clock::duration d);
		/// Resets the future offset
		delay_runner& after();

		/// Executes a function at a specific offset, ignores future_offset
		delay_runner& also_exec(std::function<void()> fn);
		/// Same as #also_exec, but occurs after future_offset
		delay_runner& exec(std::function<void()> fn);

		/// Execute a function for every frame for given duration, ignores future_offset
		///
		/// This function is called with a \c double specifying the
		/// progress (from 0 to 1)
		delay_runner& also_over(clock::duration d, std::function<void(double)> fn);
		/// Same as #also_over, but occurs after future_offset
		delay_runner& over(clock::duration d, std::function<void(double)> fn);
	};

} // namespace rt

