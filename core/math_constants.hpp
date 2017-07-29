#pragma once

/**
 * \file
 * \brief Mathematical constants
 *
 * This file provides common mathematical constants. These are calculated using
 * math functions (e.g. acos(-1) for pi), not stored constants. However, they
 * are only calculated once. All functions for these are stored in namespace
 * constant.
 */

/**
 * \namespace constant
 * \brief Mathematical constants
 *
 * This namespace contains all functions that produce non-trivial mathematical
 * constants to a great degree of accuracy.
 */

namespace constant {

	/**
	 * \fn pi
	 * \brief Pi, around 3.14159
	 *
	 * Returns Pi, the ratio of the diameter to the circumference in a circle.
	 */
	long double pi();

	/**
	 * \fn e
	 * \brief E, around 2.71828
	 *
	 * Returns E (Euler's number), the natural constant
	 */
	long double e();

	/**
	 * \fn sqrt2
	 * \brief Square root of 2, around 1.41421
	 *
	 * Returns the square root of 2, the number that becomes 2 when squared.
	 */
	long double sqrt2();

} // namespace constant
