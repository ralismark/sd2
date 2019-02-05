#pragma once

#include "include/preproc.hpp"

#ifdef _MSC_VER
// define EXPORTS if you want to compile to a dll
#ifdef EXPORTS
	// create dll
	#define _dll_api_ extern __declspec(dllexport)
	#define _dll_lib_(f) /* nil */
#else
	// use dll
	#define _dll_api_ extern __declspec(dllimport)
	#define _dll_lib_(f) _pp_link(f)
#endif
#else
#define _dll_api_ extern
#define _dll_lib_(f) /* nil */
#endif
