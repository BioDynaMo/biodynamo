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

# BioDynaMo configuration file for external projects
#
# Script defines the following variables
#   BioDynaMo_FOUND      - set to true of BioDynaMo has been found
#   BDM_USE_FILE         - cmake script that will setup the build
#
# If an external project can't locate this file emitting the following error
# message:
#      Could not find a package configuration file provided by "BioDynaMo" with
#        any of the following names:
#
#          BioDynaMoConfig.cmake
#          biodynamo-config.cmake
#
# make sure the folder containing this file is in one of the folders described
# in https://cmake.org/cmake/help/latest/command/find_package.html where
# '<prefix>' are all directories inside  PATH environment variable.
# Use`cmake -DCMAKE_FIND_DEBUG_MODE=ON ..` to debug the issue.

find_file(BDM_USE_FILE
          UseBioDynaMo.cmake
          PATHS "$ENV{BDMSYS}/share/cmake")

if(NOT BDM_USE_FILE AND BioDynaMo_FIND_REQUIRED)
  message(FATAL_ERROR "BioDynaMo not found! Try to set BDMSYS environment variable \
                       pointing to the directory that contains the file share/cmake/UseBioDynaMo.cmake \
                       or execute 'source <path-to-bdm-installation>/bin/thisbdm.sh'")
elseif(NOT BDM_USE_FILE AND NOT BioDynaMo_FIND_QUIETLY)
  message(WARNING "BioDynaMo not found! Try to set BDMSYS environment variable \
                   pointing to the directory that contains the file share/cmake/UseBioDynaMo.cmake \
                   or execute 'source <path-to-bdm-installation>/bin/thisbdm.sh'")
endif()

set(BioDynaMo_FOUND TRUE)
