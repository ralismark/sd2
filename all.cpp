/* -*- cpp.doxygen -*- */
/**
 * \file all.cpp
 * \brief Include of all files
 *
 * This is a test file which includes all files (both header and
 * implementation) in the project. This ensures that all are correct and
 * tested, and that compile-time errors (e.g. syntax) are caught. Additionally,
 * all headers are included twice to check for include guards.
 *
 * Note: Including the implementation may not be necessary (since we generate a
 *       library from them), but ensures that all files are checked. However,
 *       files with exported variables (e.g. core/runtime.cpp, which exports
 *       rt::frame) must be included (as no libraries are linked)
 */

// source
#ifdef INCLUDE_SOURCE

	#include "core/delay.cpp"
	#include "core/event.cpp"
	#include "core/math_constants.cpp"
	#include "core/opts.cpp"
	#include "core/runtime.cpp"
	#include "core/time.cpp"

	#include "disp/line.cpp"
	#include "disp/window.cpp"

	#include "input/button.cpp"
	#include "input/keystate.cpp"

	#include "res/except.cpp"
	#include "res/memblk.cpp"
	#include "res/memfile.cpp"

#endif

// headers
#include "core/delay.hpp"
#include "core/event.hpp"
#include "core/math_constants.hpp"
#include "core/opts.hpp"
#include "core/runtime.hpp"
#include "core/time.hpp"

#include "disp/line.hpp"
#include "disp/window.hpp"

#include "include/fmt.hpp"
#include "include/optional.hpp"
#include "include/preproc.hpp"
#include "include/priority_list.hpp"
#include "include/randutils.hpp"
#include "include/sigslots.hpp"
#include "include/types.hpp"
#include "include/vector.hpp"
#include "include/win32.hpp"

#include "input/button.hpp"
#include "input/keystate.hpp"

#include "res/dllport.hpp"
#include "res/except.hpp"
#include "res/memblk.hpp"
#include "res/memfile.hpp"
#include "res/resource.hpp"

// invert INCLUDE_SOURCE
#ifdef INCLUDE_SOURCE
	void initial()
	{
	}

	#undef INCLUDE_SOURCE
#else
	#define INCLUDE_SOURCE 1
#endif

// logic for repeat include
#ifndef REPEAT_RUN
	#define REPEAT_RUN 1

	#include "all.cpp"

#endif


