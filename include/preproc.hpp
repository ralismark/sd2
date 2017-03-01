#pragma once

#include "config.hpp"

#define _pp_concat_(a, b) a##b
#define _pp_concat(a, b) _pp_concat_(a, b)

#define _pp_stringify_(s) #s
#define _pp_stringify(s) _pp_stringify_(s)

#if !defined(COMPILER_MSVC)
	#define _pp_pragma(s) _Pragma(#s)
#else
	#define _pp_pragma(s) __pragma(s)
#endif

// e.g. _pp_link("user32.lib")
#define _pp_link(s) _pp_pragma(comment(lib, s))

// e.g. _pp_macro_push("NOMINMAX"), _pp_macro_pop("NOMINMAX")
#define _pp_macro_push(p) _pp_pragma(push_macro(p))
#define _pp_macro_pop(p) _pp_pragma(pop_macro(p))

// warning enable and disable
#if defined(COMPILER_MSVC)
	#define _pp_warn_push() _pp_pragma(warning(push))
	#define _pp_warn_pop() _pp_pragma(warning(pop))
	#define _pp_warn_stop(w) _pp_pragma(warning(disable : w))
#elif defined(COMPILER_GCC)
	#define _pp_warn_push() _pp_pragma(GCC diagnostic push)
	#define _pp_warn_pop() _pp_pragma(GCC diagnostic pop)
	#define _pp_warn_stop(w) _pp_pragma(GCC diagnostic ignored w)
#elif defined(COMPILER_CLANG)
	#define _pp_warn_push() _pp_pragma(clang diagnostic push)
	#define _pp_warn_pop() _pp_pragma(clang diagnostic pop)
	#define _pp_warn_stop(w) _pp_pragma(clang diagnostic ignored w)
#elif defined(COMPILER_ICC)
	// ???
	// what is it supposed to be?
	// do nothing until we figure it out
	#define _pp_warn_push() /**/
	#define _pp_warn_pop() /**/
	#define _pp_warn_stop(w) /**/
#else
	#error "No compiler is detected"
#endif
