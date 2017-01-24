#pragma once

/*
 * defaults for program configuration
 * define beforehand to override
 */

// CFG_WINNAME: window title: "<name>"
#ifndef CFG_WINNAME
#define CFG_WINNAME "Standard Window"
#endif

// CFG_WINSTYLE: window style: <sf::Style::*> | ...
#ifndef CFG_WINSTYLE
#define CFG_WINSTYLE sf::Style::Titlebar | sf::Style::Close
#endif

// CFG_WINFPS: window FPS: <number>
#ifndef CFG_WINFPS
#define CFG_WINFPS 30
#endif

// CFG_WINSIZE: window size: <x>, <y>
#ifndef CFG_WINSIZE
#define CFG_WINSIZE 1024, 600
#endif
