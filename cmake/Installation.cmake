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

# Reduce the amount of install messages (especially if we need
# to install ROOT, Paraview and Qt).
if (NOT verbose)
    set(CMAKE_INSTALL_MESSAGE NEVER)
endif()

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
        DESTINATION .
        USE_SOURCE_PERMISSIONS)

install(TARGETS biodynamo
        LIBRARY
        DESTINATION ./biodynamo/lib)

if(test)
    install(TARGETS runBiodynamoTestsMain
            RUNTIME
            DESTINATION ./biodynamo/bin)
endif()

if(${ParaView_FOUND})
    install(TARGETS BDMGlyphFilter
            LIBRARY
            DESTINATION ./biodynamo/lib/pv_plugin)
endif()

# Install third party directories
install(DIRECTORY ${CMAKE_BIODYNAMO_ROOT}/third_party
        DESTINATION .
        USE_SOURCE_PERMISSIONS
        OPTIONAL
        PATTERN "*.tar.gz" EXCLUDE)

# Copy the environment file
install(FILES ${CMAKE_BIODYNAMO_ROOT}/biodynamo-env.sh
        DESTINATION .
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
        GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)