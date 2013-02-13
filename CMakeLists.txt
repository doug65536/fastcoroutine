# Copyright 2013 A. Douglas Gale
# Permission is granted to use the fastcoroutine implementation
# for any use (including commercial use), provided this copyright
# notice is present in the product source code.

cmake_minimum_required(VERSION 2.8)
project(fastcoroutine)

message("SystemProcessor=${CMAKE_SYSTEM_PROCESSOR}")

enable_language(ASM)
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -Wa,--msyntax=att -g")

set(CMAKE_LINK_FLAGS "${CMAKE_LINK_FLAGS} -g")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE} -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mtune=corei7")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=core2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Library
add_library(fastcoroutine STATIC fastcoroutine.cpp csasm.S)

# Tests
add_executable(fastcoroutine_tests tests.cpp)
target_link_libraries(fastcoroutine_tests fastcoroutine)
add_test(fastcoroutine_tests fastcoroutine_tests)
