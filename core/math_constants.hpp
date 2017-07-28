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

namespace constant {

	// Pi, ratio of diameter to circumference
	long double pi();

	// E, natural constant
	long double e();

	// Square root of 2, the number that is 2 when squared.
	long double sqrt2();

} // namespace constant
