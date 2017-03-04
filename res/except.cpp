#include "except.hpp"

namespace res {

	// class invalid_key {{{

	invalid_key::invalid_key()
		: std::invalid_argument("Invalid key when loading resource")
	{
	}

	invalid_key::~invalid_key() = default;

	// }}}

	// class unavailable {{{

	unavailable::unavailable()
		: std::runtime_error("Resource unavailable")
	{
	}

	unavailable::~unavailable() = default;

	// }}}

} // namespace res
