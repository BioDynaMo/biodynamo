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

# This file contains configuration for the install step

# Reduce the amount of install messages (especially if we need
# to install ROOT, Paraview and Qt).
if (NOT verbose)
    set(CMAKE_INSTALL_MESSAGE NEVER)
endif()

# We set the default installation directory to $HOME/
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "$ENV{HOME}" CACHE PATH "The BioDynaMo installation path" FORCE)
endif()

execute_process(COMMAND cat version/bdm_shortversion OUTPUT_VARIABLE SHORT_VERSION WORKING_DIRECTORY ${CMAKE_BIODYNAMO_BUILD_ROOT})

# Remember to also update the install directory in the installation tests
set(DIRNAME "biodynamo-v${SHORT_VERSION}")

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
              ${CMAKE_BIODYNAMO_BUILD_ROOT}/version/bdm_version
        DESTINATION ${DIRNAME})
if(openmp)
  install(FILES ${CMAKE_BIODYNAMO_BUILD_ROOT}/omp/omp.h
          DESTINATION ${DIRNAME}/include)
  install(DIRECTORY ${CMAKE_BIODYNAMO_BUILD_ROOT}/omp
          DESTINATION ${DIRNAME}/third_party
          USE_SOURCE_PERMISSIONS
          FILES_MATCHING PATTERN "*")
endif(openmp)
if (APPLE)
  install(FILES ${CMAKE_BIODYNAMO_BUILD_ROOT}/opencl/cl2.hpp
          DESTINATION ${DIRNAME}/include)
  install(DIRECTORY ${CMAKE_BIODYNAMO_BUILD_ROOT}/opencl
          DESTINATION ${DIRNAME}/third_party
          USE_SOURCE_PERMISSIONS
          FILES_MATCHING PATTERN "*")
endif()
install(DIRECTORY ${CMAKE_BIODYNAMO_BUILD_ROOT}/etc
        DESTINATION ${DIRNAME}
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "*")
if(notebooks)
  install(DIRECTORY ${CMAKE_BIODYNAMO_BUILD_ROOT}/notebook
          DESTINATION ${DIRNAME}
          USE_SOURCE_PERMISSIONS
          FILES_MATCHING PATTERN "*"
          PATTERN "*.tar.gz" EXCLUDE)
endif()

# We need to install manually these targets in order to clear their RPATH.
# They have been already copied inside the final install directory by the
# previous instruction, but their RPATH still points to files in the build
# directory. Therefore we need to "install" them again to fix this problem.
install(TARGETS biodynamo
        LIBRARY
        DESTINATION ${DIRNAME}/lib)

if(test)
    install(TARGETS biodynamo-unit-tests
            LIBRARY
            DESTINATION ${DIRNAME}/lib)
    install(TARGETS biodynamo-unit-tests-bin
            RUNTIME
            DESTINATION ${DIRNAME}/bin)
endif()

if (notebooks)
  install(DIRECTORY ${CMAKE_BIODYNAMO_BUILD_ROOT}/notebooks DESTINATION ${DIRNAME}
          FILES_MATCHING
          PATTERN "*.ipynb" PATTERN "*.h" PATTERN "*.C" PATTERN "*.html")
endif()
