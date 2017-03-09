#pragma once

#include "config.hpp"

#if defined(HAS_OPTIONAL)

#include <optional>

namespace stx {

	using std::optional;
	using std::bad_optional_access;
	using std::nullopt_t;
	static constexpr auto& nullopt = std::nullopt;
	using std::make_optional;

} // namespace stx

#elif defined(HAS_EXP_OPTIONAL)

#include <experimental/optional>

namespace stx {

	using std::experimental::optional;
	using std::experimental::bad_optional_access;
	using std::experimental::nullopt_t;
	static constexpr auto& nullopt = std::experimental::nullopt;
	using std::experimental::make_optional;

} // namespace stx

#else

#error "no <optional> or <experimental/optional> available"

#endif
