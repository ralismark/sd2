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
	 * \brief helper to do delayed/continuous actions
	 *
	 * Construct this object, then use the member functions to do things
	 * with a delay. This is nicer wrapper around rt::exec_at and
	 * rt::exec_until in time.hpp.
	 *
	 * Currently, issues arise from creating another instance of this class
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
		delay_runner(clock::duration offset = clock::duration::zero());

		// returns the current offset time for events
		clock::time_point when() const;

		// delays the current offset by d, and resettign the future
		// offset
		delay_runner& delay(clock::duration d);
		// resets the future offset
		delay_runner& after();

		// executes a function at a specific offset, ignoring
		// future_offset
		delay_runner& also_exec(std::function<void()> fn);
		// same as also_exec, but occurs after future_offset
		delay_runner& exec(std::function<void()> fn);

		// execute a function for every frame for given duration,
		// ignoring future_offset this function is called with double
		// specifying the progress (from 0 to 1)
		delay_runner& also_over(clock::duration d, std::function<void(double)> fn);
		// same as also_over, but occurs after future_offset
		delay_runner& over(clock::duration d, std::function<void(double)> fn);
	};

} // namespace rt

