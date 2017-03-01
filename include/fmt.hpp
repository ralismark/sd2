#pragma once

#include "preproc.hpp"

_pp_warn_push();

#if defined(COMPILER_CLANG)
	_pp_warn_stop("-Wexpansion-to-defined");
#endif

#define FMT_HEADER_ONLY
#include "depends/fmt/fmt/format.h"
#include "depends/fmt/fmt/ostream.h"

_pp_warn_pop();
