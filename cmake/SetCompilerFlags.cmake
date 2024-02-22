# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & University of Surrey for the benefit of the
# BioDynaMo collaboration. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional information
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------

# This file sets all compiler flags

# check if compiler supports C++17
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++17" COMPILER_SUPPORTS_CXX17)
if(NOT COMPILER_SUPPORTS_CXX17)
  message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++17 support. Please use a different C++ compiler.")
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# turn off compiler specific extensions e.g. gnu++17
set(CMAKE_CXX_EXTENSIONS OFF)

# get machine architecture
execute_process(COMMAND arch OUTPUT_VARIABLE DISTRO_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)

# general flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-missing-braces -fPIC ${OpenMP_CXX_FLAGS}")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wall -Wno-missing-braces -fPIC ${OpenMP_C_FLAGS}")
if(${DISTRO_ARCH} STREQUAL "x86_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -m64")
endif()
if(${DISTRO_ARCH} STREQUAL "aarch64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=native")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -mcpu=native")
endif()

# suppress OpenCL-generated warnings
if (OPENCL_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-ignored-attributes")
endif()

# special clang flags
if(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
  # silence clang 3.9 warning
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-undefined-var-template")
endif()

# flags for specific build type
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_OPTIMIZED      "-Ofast -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG          "-g")
set(CMAKE_CXX_FLAGS_DEBUGFULL      "-g3")
set(CMAKE_CXX_FLAGS_COVERAGE       "-O0 -g3 --coverage -fno-default-inline -fno-inline -ftest-coverage -fprofile-arcs")
set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O3 -g -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE          "-O3 -DNDEBUG")
set(CMAKE_C_FLAGS_OPTIMIZED        "-Ofast -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG            "-g")
set(CMAKE_C_FLAGS_DEBUGFULL        "-g3 -fno-inline")
set(CMAKE_C_FLAGS_COVERAGE         "-O0 -g3 --coverage -fno-inline -ftest-coverage -fprofile-arcs")

if (coverage)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUGFULL}")
endif()
