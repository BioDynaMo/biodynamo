# -----------------------------------------------------------------------------
#
# Copyright (C) 2022 CERN & Newcastle University for the benefit of the
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

cmake_minimum_required(VERSION 3.2.0)

project(multi_simulation_test)

find_package(BioDynaMo REQUIRED)
include("${BDM_USE_FILE}")
include_directories("src")
include_directories("${BDMSYS}/include")

file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE SOURCES src/*.cc)

bdm_add_executable(multi_simulation_test
                   HEADERS "${HEADERS}"
                   SOURCES "${SOURCES}"
                   LIBRARIES "${BDM_REQUIRED_LIBRARIES}")
