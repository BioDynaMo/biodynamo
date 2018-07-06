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

cmake_minimum_required(VERSION 3.2.0)

project(multiple_simulations)

find_package(BioDynaMo REQUIRED)
include("${BDM_USE_FILE}")

file(GLOB_RECURSE HEADERS *.h)
file(GLOB_RECURSE SOURCES *.cc)

bdm_add_executable(multiple_simulations
                   HEADERS "${HEADERS}"
                   SOURCES "${SOURCES}"
                   LIBRARIES "${BDM_REQUIRED_LIBRARIES}")
