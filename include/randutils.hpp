#pragma once

#include "core/preproc.hpp"

_pp_warn_push();

#if defined(COMPILER_MSVC)
	_pp_warn_stop(4307);
#elif defined(COMPILER_CLANG)
	_pp_warn_stop("-Wunused-variable");
#endif

#include "depends/randutils/randutils.hpp"

_pp_warn_pop();
