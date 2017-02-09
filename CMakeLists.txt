cmake_minimum_required(VERSION 3.5)

project(sd2)

add_subdirectory(export)

include_directories("${CMAKE_SOURCE_DIR}")
add_executable(evtest evtest.cpp)

add_executable(all_check all.cpp)
