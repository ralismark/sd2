#pragma once

#define WIN32_LEAN_AND_MEAN 1
#define NOMINMAX

// no unicode, use ANSI
#ifdef UNICODE
	#undef UNICODE
#endif

#ifdef _UNICODE
	#undef _UNICODE
#endif

// #define UNICODE
// #define _UNICODE

#include <windows.h>
