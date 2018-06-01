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
  message(FATAL_ERROR "Git not found.")
endif()

execute_process(
     COMMAND "cmake/version/generate_version_files.py" ${GIT_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}
     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_custom_target(update-version-info
     COMMAND "cmake/version/generate_version_files.py" ${GIT_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}
     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
