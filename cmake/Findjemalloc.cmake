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

# This script will set the following variables
#  JEMALLOC_FOUND
#  JEMALLOC_LIBRARY_DIR

find_library(JEMALLOC_LIBRARY_FULL_PATH NAMES libjemalloc.so)

get_filename_component(JEMALLOC_LIBRARY_DIR ${JEMALLOC_LIBRARY_FULL_PATH} DIRECTORY)

if (JEMALLOC_LIBRARY_DIR)
  set(JEMALLOC_FOUND TRUE)
  message(STATUS "Found jemalloc library ${JEMALLOC_LIBRARY_DIR}")
else()
  message(STATUS "jemalloc library not found")
endif()
