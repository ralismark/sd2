/* -*- cpp.doxygen -*- */

#include <chrono>
#include <functional>

/**
 * \file
 * \brief Delayed and repeated action base
 *
 * This provides a basic framework for running code at a delayed time, or
 * repeated calls until a specific point in time.
 *
 * This can be used independent of runtime.hpp. Set frame_now and call
 * exec_step periodically.
 *
 * A nice wrapper around this interface is available through \ref
 * rt::delayed_runner in delay.hpp.
 */

namespace rt {

	// clock type used for most things
	using clock = std::chrono::steady_clock;

	/**
	 * \var frame_now
	 * \brief Time used for exec_step
	 *
	 * This must be set before every call to exec_step(), as it is used as
	 * the 'current' time for delay purposes. This can also be used as a
	 * fixed time for the current frame/tick.
	 */
	extern clock::time_point frame_now;

	/**
	 * \fn exec_at
	 * \brief Call a function at specified point in time
	 *
	 * This sets fn to be called with a delay, only executing when
	 * #frame_now is past \p when during a call to exec_step().
	 */
	void exec_at(
		clock::time_point when, ///< [in] time to execute \a fn
		std::function<void()> fn ///< [in] function called after a delay
		);

	/**
	 * \fn exec_until
	 * \brief Execute a function every tick for a time
	 *
	 * This adds \p fn to a list of functions, and is executed every time
	 * exec_step() is called until \p when is reached.
	 */
	void exec_until(
		clock::time_point when, ///< [in] point in time to stop executing fn
		std::function<void()> fn ///< [in] function to execute every tick
		);

	/**
	 * \fn exec_step
	 * \brief Call delayed/continued callbacks
	 *
	 * Functions registered by exec_at() or exec_until() will be called
	 * here when appropritate, using time stored in #frame_now.
	 */
	void exec_step();

} // namespace rt
