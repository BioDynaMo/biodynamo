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
include_directories("$ENV{BDM_INSTALL_DIR}/third_party/ray/include")
include_directories("$ENV{BDM_INSTALL_DIR}/third_party/ray/include/common")
include_directories("$ENV{BDM_INSTALL_DIR}/third_party/ray/include/common/thirdparty")
include_directories("$ENV{BDM_INSTALL_DIR}/third_party/ray/include/ray/gcs/format")
link_directories("$ENV{BDM_INSTALL_DIR}/third_party/ray/lib")
include_directories("src")

file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE SOURCES src/*.cc)

bdm_add_executable(distributed
                   HEADERS "${HEADERS}"
                   SOURCES "${SOURCES}"
                   LIBRARIES "${BDM_REQUIRED_LIBRARIES}"
                             "-lboost_system"
                             "-larrow"
                             "-lplasma"
                             "-llocal_scheduler_client"
                             "-lcommon"
                             "-lray")
