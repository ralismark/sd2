#include "except.hpp"

namespace res {

	// class invalid_key {{{

	invalid_key::invalid_key()
		: std::invalid_argument("Invalid key when loading resource")
	{
	}

	virtual invalid_key::~invalid_key() = default;

	// }}}

	// class unavailable {{{

	unavailable::unavailable()
		: std::runtime_error("Resource unavailable")
	{
	}

	virtual unavailable::~unavailable() = default;

	// }}}

} // namespace res
