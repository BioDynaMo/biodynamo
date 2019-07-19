# -----------------------------------------------------------------------------
#
# Copyright (C) The BioDynaMo Project.
# All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional information
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------

# Check the compiler and provide it automatically if it not found
include(CheckLanguage)
check_language(CXX)
check_language(C)

if(CMAKE_CXX_COMPILER)
    # If we are on a MacOS system, we then force the compiler to
    # be LLVM's clang. This is done because standard
    # MacOS's g++ do not has OpenMP enabled.
    if (APPLE)
        if (NOT $ENV{CXX})
            UNSET(CMAKE_CXX_COMPILER CACHE)
            if (EXISTS /usr/local/opt/llvm/bin/clang++) # Brew
                SET(CMAKE_CXX_COMPILER "/usr/local/opt/llvm/bin/clang++")
                include_directories(/usr/local/opt/llvm/include)
                link_directories(/usr/local/opt/llvm/lib)
            elseif(EXISTS /opt/local/bin/clang++-mp-8.0) # MacPort
                SET(CMAKE_CXX_COMPILER "/opt/local/bin/clang++-mp-8.0")
                include_directories(/opt/local/include/)
                link_directories(/opt/local/lib/)
            elseif(EXISTS /sw/opt/llvm-5.0/bin/clang++) # Fink
                SET(CMAKE_CXX_COMPILER "/sw/opt/llvm-5.0/bin/clang++")
                include_directories(/sw/include)
                link_directories(/sw/opt/llvm-5.0/lib/)
                link_directories(/sw/lib)
            else()
                MESSAGE(FATAL_ERROR "No suitable C++ compiler compatible with OpenMP was found in your system! In order to compile BioDynaMo \
you are required to install one. Please run prerequisites.sh script to install it. You can also specify your custom \
compiler by exporting the CXX environmental variable (e.g. export CXX=/path/to/my/compiler/g++ && cmake ../).")
            endif()
        endif()
    endif()
    enable_language(CXX)
else()
    # Manually set the g++ compiler that we need for CentOS.
    # We also set the various link directories needed.
    IF(${DETECTED_OS} MATCHES "centos.*")
        if (NOT $ENV{CXX})
            UNSET(CMAKE_CXX_COMPILER CACHE)
            SET(CMAKE_CXX_COMPILER "/opt/rh/devtoolset-7/root/usr/bin/g++")
            link_directories(/opt/rh/devtoolset-7/root/usr/lib)
            link_directories(/opt/rh/devtoolset-7/root/usr/lib/dyninst)
            link_directories(/opt/rh/devtoolset-7/root/usr/lib64)
            link_directories(/opt/rh/devtoolset-7/root/usr/lib64/dyninst)
        endif()
        if (NOT $ENV{C})
            UNSET(CMAKE_C_COMPILER CACHE)
            SET(CMAKE_C_COMPILER "/opt/rh/devtoolset-7/root/usr/bin/gcc")
        endif()
        enable_language(CXX)
    endif()
endif()

# Check if we found a C compiler.
if(CMAKE_C_COMPILER)
    if (APPLE)
        if (NOT $ENV{C})
            UNSET(CMAKE_C_COMPILER CACHE)
            if (EXISTS /usr/local/opt/llvm/bin/clang) # Brew
                SET(CMAKE_C_COMPILER "/usr/local/opt/llvm/bin/clang")
                include_directories(/usr/local/opt/llvm/include)
                link_directories(/usr/local/opt/llvm/lib)
            elseif(EXISTS /opt/local/bin/clang-mp-8.0) # MacPort
                SET(CMAKE_CXX_COMPILER "/opt/local/bin/clang-mp-8.0")
                include_directories(/opt/local/include/)
                link_directories(/opt/local/lib/)
            elseif(EXISTS /sw/opt/llvm-5.0/bin/clang++) # Fink
                SET(CMAKE_CXX_COMPILER "/sw/opt/llvm-5.0/bin/clang")
                include_directories(/sw/include)
                link_directories(/sw/opt/llvm-5.0/lib/)
                link_directories(/sw/lib)
            else()
                MESSAGE(FATAL_ERROR "No suitable C compiler was found in your system! In order to compile BioDynaMo \
    you are required to install one. Please run prerequisites.sh script to install it. You can also specify your custom \
    compiler by exporting the C environmental variable (e.g. export C=/path/to/my/compiler/gcc && cmake ../).")
            endif()

        endif()
    endif()
    enable_language(C)
else()
    # Force the
    IF(${DETECTED_OS} MATCHES "centos.*")
        if (NOT $ENV{C})
            UNSET(CMAKE_C_COMPILER CACHE)
            SET(CMAKE_C_COMPILER "/opt/rh/devtoolset-7/root/usr/bin/gcc")
        endif()
    endif()
    enable_language(C)
endif()
