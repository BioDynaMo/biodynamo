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

project(distributed)

find_package(BioDynaMo REQUIRED)
include("${BDM_USE_FILE}")
include_directories("/home/bdm/ray/src")
include_directories("/home/bdm/ray/src/common")
include_directories("/home/bdm/ray/src/common/thirdparty")
include_directories("/home/bdm/ray/src/ray/gcs/format")
include_directories("/home/bdm/ray/thirdparty/pkg/arrow/cpp/build/cpp-install/include")
include_directories("/home/bdm/ray/thirdparty/pkg/boost/include")
include_directories("/home/bdm/ray/thirdparty/pkg/flatbuffers/include")
link_directories("/home/bdm/ray/build/src/local_scheduler")
link_directories("/home/bdm/ray/build/src/common")
link_directories("/home/bdm/ray/build/src/ray")
link_directories("/home/bdm/ray/thirdparty/pkg/boost/lib")
link_directories("/home/bdm/ray/thirdparty/pkg/arrow/cpp/build/cpp-install/lib")
include_directories("src")

file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE SOURCES src/*.cc)

bdm_add_executable(distributed
                   HEADERS "${HEADERS}"
                   SOURCES "${SOURCES}"
                   LIBRARIES "${BDM_REQUIRED_LIBRARIES}" "-lboost_system" "-lray" "-lplasma" "-larrow" "-llocal_scheduler_client" "-lcommon")
