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

# This script will set the following variables
#  NUMA_FOUND - System has NUMA
#  NUMA_INCLUDE_DIR - NUMA include directory
#  NUMA_LIBRARY_DIR - NUMA library directory
#  NUMA_LIBRARIES - The libraries needed to use NUMA

find_path(NUMA_INCLUDE_DIR NAMES numa.h)
find_library(NUMA_LIBRARY_DIR NAMES numa)

if (NUMA_LIBRARY_DIR)
  set(NUMA_FOUND TRUE)
  set(NUMA_LIBRARIES numa)
  message(STATUS "Found NUMA library ${NUMA_LIBRARY_DIR}")
else()
  message(STATUS "NUMA library not found")
endif()
