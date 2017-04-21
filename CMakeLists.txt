cmake_minimum_required(VERSION 3.5)
include(CheckIncludeFileCXX)

project(sd2)

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/h ${PROJECT_BINARY_DIR}/g)
include_directories(${PROJECT_BINARY_DIR}/h)

# output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# C++17 support
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" OR "${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
	# using Visual Studio C++
	set(CPP17_FLAG "/std:c++latest")
	add_compile_options("/EHsc")
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

# sfml
find_library(LIB_SFML_AUDIO sfml-audio)
find_library(LIB_SFML_GRAPHICS sfml-graphics)
find_library(LIB_SFML_NETWORK sfml-network)
find_library(LIB_SFML_SYSTEM sfml-system)
find_library(LIB_SFML_WINDOW sfml-window)
add_library(sfml INTERFACE)
target_link_libraries(sfml INTERFACE
		${LIB_SFML_AUDIO} ${LIB_SFML_GRAPHICS} ${LIB_SFML_NETWORK} ${LIB_SFML_SYSTEM} ${LIB_SFML_WINDOW}
	)

# entityx
set(ENTITYX_BUILD_SHARED false CACHE BOOL "" FORCE)
set(ENTITYX_BUILD_TESTING false CACHE BOOL "" FORCE)
add_subdirectory(depends/entityx)

# headers
include_directories("${PROJECT_SOURCE_DIR}")

# libraries
add_subdirectory(core)
add_subdirectory(disp)
add_subdirectory(input)
add_subdirectory(res)
add_subdirectory(export)

# runtime

add_library(runtime INTERFACE)
target_link_libraries(runtime INTERFACE core disp input res)
target_link_libraries(runtime INTERFACE sfml)

# source

add_executable(cvt-export cvt-export.cpp)
target_link_libraries(cvt-export res)

add_executable(cvt-wrapper cvt-wrapper.cpp)

function(res_export libname)
	add_custom_command(
		OUTPUT ${PROJECT_BINARY_DIR}/g/${libname}_lib.cpp ${PROJECT_BINARY_DIR}/g/${libname}_lib.hpp
		COMMAND $<TARGET_FILE:cvt-export> ${PROJECT_BINARY_DIR}/g/${libname}_lib.cpp ${PROJECT_BINARY_DIR}/g/${libname}_lib.hpp ${libname}_res ${ARGN}
		DEPENDS cvt-export ${ARGN}
		)
	add_library(${libname}_res SHARED ${PROJECT_BINARY_DIR}/g/${libname}_lib.cpp)

	add_custom_command(
		OUTPUT ${PROJECT_BINARY_DIR}/g/${libname}_wrap.cpp ${PROJECT_BINARY_DIR}/h/${libname}.hpp
		COMMAND $<TARGET_FILE:cvt-wrapper> ${PROJECT_BINARY_DIR}/g/${libname}_wrap.cpp ${PROJECT_BINARY_DIR}/h/${libname}.hpp export ${PROJECT_BINARY_DIR}/g/${libname}_lib.hpp ${ARGN}
		DEPENDS cvt-export
		)
	add_library(${libname} ${PROJECT_BINARY_DIR}/g/${libname}_wrap.cpp)
	target_link_libraries(${libname} ${libname}_res)
endfunction()

add_executable(evtest evtest.cpp)
target_link_libraries(evtest runtime)

add_executable(all_check all.cpp)
target_link_libraries(all_check sfml)

add_executable(sort sort.cpp)
target_link_libraries(sort -subsystem:windows -entry:mainCRTStartup)
target_link_libraries(sort resource)
target_link_libraries(sort runtime)

add_executable(stars stars.cpp)
target_link_libraries(stars -subsystem:windows -entry:mainCRTStartup)
target_link_libraries(stars runtime)

add_executable(estars entity.cpp)
target_link_libraries(estars runtime entityx)


