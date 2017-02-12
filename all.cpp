// this file contains all includes
// this ensures that all files are syntactically correct
// additionally, this ensures that all headers are correctly guarded against multiple inclusions

#ifndef ALL_INCLUDE
	#define ALL_INCLUDE 1
#else
	#undef ALL_INCLUDE
	#define ALL_INCLUDE 2
#endif

// all includes

#include "core/event.hpp"
#include "core/preproc.hpp"
#include "core/priority_list.hpp"
#include "core/runtime.cpp"
#include "core/sigslots.hpp"
#include "core/types.hpp"

#include "disp/window.cpp"

#include "export/dllport.hpp"
// #include "export/resource.cpp" // this is independent from the rest
#include "export/resource.hpp"

#include "include/fmt.hpp"
#include "include/optional.hpp"
#include "include/randutils.hpp"
#include "include/vector.hpp"
#include "include/win32.hpp"

#include "input/button.cpp"
#include "input/keystate.cpp"

// re-include everything
#if ALL_INCLUDE < 2
// first run

void initial() {} // required to be defined, from core/runtime

#include "all.cpp"
#endif