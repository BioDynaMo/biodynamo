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

# This file contains configuration for the install step

# check if CMAKE_INSTALL_PREFIX is empty or /opt/biodynamo
# We only allow /opt/biodynamo as prefix path. This enables us to provide
# one development environment script for building biodynamo and out of source
# simulations. (The path to the development install is known and can be
# hardcoded )
if (NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX ${BDM_INSTALL_DIR} CACHE PATH "BioDynaMo install prefix" FORCE)
elseif(CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
  set(CMAKE_INSTALL_PREFIX ${BDM_INSTALL_DIR} CACHE PATH "BioDynaMo install prefix" FORCE)
elseif( CMAKE_INSTALL_PREFIX AND NOT CMAKE_INSTALL_PREFIX STREQUAL "${BDM_INSTALL_DIR}" )
  message(FATAL_ERROR "CMAKE_INSTALL_PREFIX must be ${BDM_INSTALL_DIR}")
endif()

# Install biodynamo in its final directory
install(DIRECTORY ${CMAKE_BIODYNAMO_ROOT}/biodynamo
        DESTINATION .)

# Install third party directories
install(DIRECTORY ${CMAKE_BIODYNAMO_ROOT}/third_party
        DESTINATION .
        OPTIONAL
        PATTERN "*.tar.gz" EXCLUDE
        PATTERN "root/bin" EXCLUDE
        PATTERN "paraview/bin" EXCLUDE
        PATTERN "qt/bin" EXCLUDE)

# Copy root executables and make them executable
install(DIRECTORY ${CMAKE_BIODYNAMO_ROOT}/third_party/root/bin
        DESTINATION third_party/root
        OPTIONAL
        FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
        GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        PATTERN "${CMAKE_BIODYNAMO_ROOT}/third_party/root/bin/*")

# Copy paraviews executable and make them executable
install(DIRECTORY ${CMAKE_BIODYNAMO_ROOT}/third_party/paraview/bin
        DESTINATION third_party/paraview
        OPTIONAL
        FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
        GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        PATTERN "${CMAKE_BIODYNAMO_ROOT}/third_party/paraview/bin/*")

# Copy qt executable and make them executable
install(DIRECTORY ${CMAKE_BIODYNAMO_ROOT}/third_party/qt/bin
        DESTINATION third_party/qt
        OPTIONAL
        FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
        GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        PATTERN "${CMAKE_BIODYNAMO_ROOT}/third_party/qt/bin/*"
        )

# Copy the environment file
install(FILES ${CMAKE_BIODYNAMO_ROOT}/biodynamo-env.sh
        DESTINATION .)