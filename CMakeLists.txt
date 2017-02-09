cmake_minimum_required(VERSION 3.5)

project(sd2)

# C++17 support
if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"))
	# using Clang or GCC
	set(CPP17_FLAG "-std=c++1z")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# using Visual Studio C++
	set(CPP17_FLAG "/std:c++latest")
else()
	# includes Intel C++
	message(FATAL_MESSAGE "Compiler not supported, but may be in the future")
endif()
add_compile_options(${CPP17_FLAG})

# source
add_subdirectory(export)

include_directories("${CMAKE_SOURCE_DIR}")
add_executable(evtest evtest.cpp)

add_executable(all_check all.cpp)
