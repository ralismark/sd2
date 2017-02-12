cmake_minimum_required(VERSION 3.5)
include(CheckIncludeFileCXX)

project(sd2)

# C++17 support
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" OR "${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
	# using Visual Studio C++
	set(CPP17_FLAG "/std:c++latest")
elseif(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"))
	# using Clang or GCC
	set(CPP17_FLAG "-std=c++1z")
else()
	# includes Intel C++
	message(FATAL_MESSAGE "Compiler not supported, but may be in the future")
endif()
add_compile_options(${CPP17_FLAG})

# CRT warnings
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# config header
set(CMAKE_REQUIRED_FLAGS ${CPP17_FLAG})

check_include_file_cxx(optional HAS_OPTIONAL)
if(NOT HAS_OPTIONAL)
	check_include_file_cxx(experimental/optional HAS_EXP_OPTIONAL)
	if(NOT HAS_EXP_OPTIONAL)
		message(WARNING "<optional> and <experimental/optional> not available")
	endif()
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(COMPILER_CLANG 1)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(COMPILER_GCC 1)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(COMPILER_MSVC 1)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	set(COMPILER_ICC 1)
else()
	message(FATAL_MESSAGE "Unknown compiler")
endif()

configure_file(
	"${PROJECT_SOURCE_DIR}/config.hpp.in"
	"${PROJECT_BINARY_DIR}/h/config.hpp"
)
include_directories("${PROJECT_BINARY_DIR}/h")

# source
include_directories("${PROJECT_SOURCE_DIR}")

add_subdirectory(export)

add_executable(evtest evtest.cpp)
add_executable(all_check all.cpp)
add_executable(sort sort.cpp)