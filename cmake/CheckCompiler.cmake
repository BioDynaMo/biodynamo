# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

# Check the compiler and provide it automatically if it not found
include(CheckLanguage)
check_language(CXX)
check_language(C)

if(CMAKE_CXX_COMPILER)
    enable_language(CXX)
else()
    # Manually set the g++ compiler that we need for CentOS.
    # We also set the various link directories needed.
    IF(${DETECTED_OS} MATCHES "centos.*")
        if (NOT DEFINED ENV{CXX})
            UNSET(CMAKE_CXX_COMPILER CACHE)
            SET(CMAKE_CXX_COMPILER "/opt/rh/devtoolset-7/root/usr/bin/g++")
            link_directories(/opt/rh/devtoolset-7/root/usr/lib)
            link_directories(/opt/rh/devtoolset-7/root/usr/lib/dyninst)
            link_directories(/opt/rh/devtoolset-7/root/usr/lib64)
            link_directories(/opt/rh/devtoolset-7/root/usr/lib64/dyninst)
        endif()
        if (NOT DEFINED ENV{CC})
            UNSET(CMAKE_C_COMPILER CACHE)
            SET(CMAKE_C_COMPILER "/opt/rh/devtoolset-7/root/usr/bin/gcc")
        endif()
        enable_language(CXX)
    endif()
endif()

# Check if we found a C compiler.
if(CMAKE_C_COMPILER)
    enable_language(C)
else()
    # Force the
    IF(${DETECTED_OS} MATCHES "centos.*")
        if (NOT DEFINED ENV{CC})
            UNSET(CMAKE_C_COMPILER CACHE)
            SET(CMAKE_C_COMPILER "/opt/rh/devtoolset-7/root/usr/bin/gcc")
        endif()
    endif()
    enable_language(C)
endif()
