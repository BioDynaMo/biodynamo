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

execute_process(COMMAND git describe --tags OUTPUT_VARIABLE VERSION WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

# This regex extracts whatever is before the first "-" character, which should be
# the version number in the form vMAJOR.MINOR.PATCH
STRING(REGEX MATCH "[^-]*" SHORT_VERSION ${VERSION})

# TODO(ahmad): see https://trello.com/c/ZT8iHKky
# set(DIRNAME "biodynamo-${SHORT_VERSION}")
set(DIRNAME "bdm")

# Install biodynamo in its final directory
install(DIRECTORY ${CMAKE_BIODYNAMO_BUILD_ROOT}/bin
        DESTINATION ${DIRNAME}
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "*"
        PATTERN "*.tar.gz" EXCLUDE)
install(DIRECTORY ${CMAKE_BIODYNAMO_BUILD_ROOT}/demo
        DESTINATION ${DIRNAME}
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "*"
        PATTERN "*.tar.gz" EXCLUDE)
install(DIRECTORY ${CMAKE_BIODYNAMO_BUILD_ROOT}/doc
        DESTINATION ${DIRNAME}
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "*"
        PATTERN "*.tar.gz" EXCLUDE)
install(DIRECTORY ${CMAKE_BIODYNAMO_BUILD_ROOT}/include
        DESTINATION ${DIRNAME}
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "*"
        PATTERN "*.tar.gz" EXCLUDE)
install(DIRECTORY ${CMAKE_BIODYNAMO_BUILD_ROOT}/lib
        DESTINATION ${DIRNAME}
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "*"
        PATTERN "*.tar.gz" EXCLUDE)
install(DIRECTORY ${CMAKE_BIODYNAMO_BUILD_ROOT}/share
        DESTINATION ${DIRNAME}
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "*"
        PATTERN "*.tar.gz" EXCLUDE)
install(DIRECTORY ${CMAKE_BIODYNAMO_BUILD_ROOT}/simulation-template
        DESTINATION ${DIRNAME}
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "*"
        PATTERN "*.tar.gz" EXCLUDE)
install(DIRECTORY ${CMAKE_BIODYNAMO_BUILD_ROOT}/third_party
        DESTINATION ${DIRNAME}
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "*"
        PATTERN "*.tar.gz" EXCLUDE)
install(FILES ${CMAKE_BIODYNAMO_BUILD_ROOT}/LICENSE
              ${CMAKE_BIODYNAMO_BUILD_ROOT}/NOTICE
        DESTINATION ${DIRNAME})

# We need to install manually these targets in order to clear their RPATH.
# They have been already copied inside the final install directory by the
# previous instruction, but their RPATH still points to files in the build
# directory. Therefore we need to "install" them again to fix this problem.
install(TARGETS biodynamo
        LIBRARY
        DESTINATION ${DIRNAME}/lib)

if(test)
    install(TARGETS biodynamo-unit-tests
            RUNTIME
            DESTINATION ${DIRNAME}/bin)
endif()

if(${ParaView_FOUND})
    install(TARGETS BDMGlyphFilter
            LIBRARY
            DESTINATION ${DIRNAME}/lib/pv_plugin)
endif()
