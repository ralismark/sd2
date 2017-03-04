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
 *       library from them), but ensures that all files are checked.
 */

#ifndef EXPORTS
	#define EXPORTS 1 // for export/resource
#endif

// source
#ifdef INCLUDE_SOURCE

	#include "core/event.cpp"
	#include "core/opts.cpp"
	#include "core/runtime.cpp"

	#include "disp/window.cpp"

	#include "export/resource.cpp"

	#include "input/button.cpp"
	#include "input/keystate.cpp"

	#include "res/except.cpp"
	#include "res/memblk.cpp"
	#include "res/memfile.cpp"

#endif

// headers
#include "core/event.hpp"
#include "core/opts.hpp"
#include "core/runtime.hpp"

#include "disp/window.hpp"

#include "export/dllport.hpp"
#include "export/resource.hpp"

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

	#include __FILE__

#endif


