#pragma once

#if defined(_MSC_VER) && !defined(__clang__)
	#define _pp_pragma(s) _Pragma(#s)
#else
	#define _pp_pragma(s) __pragma(s)
#endif

// e.g. _pp_link("user32.lib")
#define _pp_link(s) _pp_pragma(comment(lib, s))
