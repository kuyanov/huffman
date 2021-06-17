project(Tests)
cmake_minimum_required(VERSION 3.10)

SET(CMAKE_CXX_STANDARD 17)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(../googletest build)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

find_library(HuffmanLirary ../lib)

add_executable(Tests_run main_tests.cpp student_tests.cpp)
target_link_libraries(Tests_run HuffmanLibrary gtest gtest_main)
