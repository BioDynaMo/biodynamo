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

if(NOT GIT_FOUND)
  message("Git was not found on your system. Therefore the detection of the current version of BioDynaMo will
be done statically. In order to enable the automatic detection, please run prerequisites.sh script and then cmake again.")
endif()

execute_process(
     COMMAND ${Python_EXECUTABLE} util/version/generate_version_files.py ${GIT_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}
             ${PROJECT_VERSION} ${CMAKE_SOURCE_DIR}/.git
     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

# Set CMake variable that can be consumed by other build scripts
execute_process(COMMAND cat version/bdm_version
                OUTPUT_VARIABLE VERSION
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set_property(DIRECTORY APPEND
    PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/.git/index")
