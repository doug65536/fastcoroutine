cmake_minimum_required(VERSION 2.8)
project(fastcoroutine)

message("SystemProcessor=${CMAKE_SYSTEM_PROCESSOR}")

enable_language(ASM)
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -Wa,--msyntax=att -g")
set(CMAKE_LINK_FLAGS "${CMAKE_ASM_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

#if(NOT EXISTS ml64.exe)
#  set(ml64_locations
#    "$ENV{ProgramFiles(x86)}/Microsoft Visual Studio 10.0/VC/bin/amd64"
#    "$ENV{ProgramFiles(x86)}/Microsoft Visual Studio 10.0/VC/bin/x86_amd64"
#    "$ENV{ProgramFiles}/Microsoft Visual Studio 10.0/VC/bin/amd64"
#    "$ENV{ProgramFiles}/Microsoft Visual Studio 10.0/VC/bin/x86_amd64"
#  )
#  foreach(place in ml64_locations)
#    if(EXISTS "${place}")
#      set(CMAKE_ASMMASM_COMPILER_ENV_VAR ML64)
#      set(ENV{ML64} ${place})
#    endif()
#  endforeach()
#endif()
#set(CMAKE_ASMMASM_COMPILER ml64.exe)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_executable(contextswitch contextswitch.cpp csasm.S)
