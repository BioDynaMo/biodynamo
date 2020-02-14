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

if(NOT GIT_FOUND)
  message("Git was not found on your system. Therefore the detection of the current version of BioDynaMo will
be done statically. In order to enable the automatic detection, please run prerequisites.sh script and then cmake again.")
endif()

execute_process(
     COMMAND ${PYTHON_EXECUTABLE} util/version/generate_version_files.py ${GIT_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}
             ${PROJECT_VERSION} ${CMAKE_SOURCE_DIR}/.git
     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_property(DIRECTORY APPEND
    PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/.git/index")
