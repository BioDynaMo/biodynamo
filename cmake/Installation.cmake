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
  set(CMAKE_INSTALL_PREFIX $ENV{BDM_INSTALL_DIR} CACHE PATH "BioDynaMo install prefix" FORCE)
elseif(CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
  set(CMAKE_INSTALL_PREFIX $ENV{BDM_INSTALL_DIR} CACHE PATH "BioDynaMo install prefix" FORCE)
elseif( CMAKE_INSTALL_PREFIX AND NOT CMAKE_INSTALL_PREFIX STREQUAL "$ENV{BDM_INSTALL_DIR}" )
  message(FATAL_ERROR "CMAKE_INSTALL_PREFIX must be $ENV{BDM_INSTALL_DIR}")
endif()

# set install directories
set(CMAKE_INSTALL_ROOT          "biodynamo"                      CACHE PATH "Install root")
set(CMAKE_INSTALL_BINDIR        "${CMAKE_INSTALL_ROOT}/bin"      CACHE PATH "User executables (bin)")
set(CMAKE_INSTALL_INCLUDEDIR    "${CMAKE_INSTALL_ROOT}/include"  CACHE PATH "C/C++ header files (include)")
set(CMAKE_INSTALL_LIBDIR        "${CMAKE_INSTALL_ROOT}/lib"      CACHE PATH "Object code libraries (lib)")
set(CMAKE_INSTALL_PVPLUGINDIR   "${CMAKE_INSTALL_ROOT}/lib/pv_plugin" CACHE PATH "ParaView libraries") # Must be in separate dir!
set(CMAKE_INSTALL_CMAKEDIR      "${CMAKE_INSTALL_ROOT}/cmake"    CACHE PATH "CMake files required from external projects")
set(CMAKE_INSTALL_DATADIR       "${CMAKE_INSTALL_ROOT}/share"    CACHE PATH "Read-only architecture-independent data (share)")
set(CMAKE_INSTALL_CMAKEDATADIR  "${CMAKE_INSTALL_DATADIR}/cmake" CACHE PATH "Build related files (DATADIR/cmake)")

# hide them from configuration tools
mark_as_advanced(${CMAKE_INSTALL_ROOT}
                 ${CMAKE_INSTALL_BINDIR}
                 ${CMAKE_INSTALL_INCLUDEDIR}
                 ${CMAKE_INSTALL_LIBDIR}
                 ${CMAKE_INSTALL_CMAKEDIR}
                 ${CMAKE_INSTALL_DATADIR}
                 ${CMAKE_INSTALL_CMAKEDATADIR})

# TODO(lukas) add logic to detect correct env script (distinguishing LINUX and
# APPLE might not be enough in the future)
if(LINUX)
  configure_file(util/installation/common/biodynamo-linux-env.sh ${CMAKE_CURRENT_BINARY_DIR}/biodynamo-env.sh @ONLY)
elseif(APPLE)
  configure_file(util/installation/common/biodynamo-macos-env.sh ${CMAKE_CURRENT_BINARY_DIR}/biodynamo-env.sh @ONLY)
endif()
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/biodynamo-env.sh DESTINATION .)
# biodynamo cli
install(FILES cli/biodynamo.py DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME biodynamo
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install(DIRECTORY cli/ DESTINATION ${CMAKE_INSTALL_BINDIR}
        FILES_MATCHING PATTERN "*.py")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/version/version.py"
        DESTINATION ${CMAKE_INSTALL_BINDIR})
# bdm-config
install(FILES util/makefile-build/bdm-config DESTINATION ${CMAKE_INSTALL_BINDIR}
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
# bdm-code-generation
install(FILES util/makefile-build/bdm-code-generation DESTINATION ${CMAKE_INSTALL_BINDIR}
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
# libbiodynamo.so
install(TARGETS biodynamo LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES build/libbiodynamo_dict_rdict.pcm DESTINATION ${CMAKE_INSTALL_LIBDIR})
# libbdmcuda.a
if(CUDA_FOUND)
  install(TARGETS bdmcuda ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} OPTIONAL)
endif()
# headers and python scripts
install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h" PATTERN "*.cl" PATTERN "*.py")
install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.py"
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/version/version.h"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
#   third party headers
file(GLOB MPARK_HEADERS ${CMAKE_BINARY_DIR}/mpark/mpark/*)
install(FILES ${MPARK_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mpark)
install(FILES third_party/cpp_magic.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES third_party/OptionParser.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES third_party/cpptoml/cpptoml.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cpptoml)
# build files
file(GLOB SELECTION_FILES cmake/*.xml)
install(FILES ${SELECTION_FILES} DESTINATION ${CMAKE_INSTALL_CMAKEDATADIR})
install(FILES cmake/BioDynaMo.cmake DESTINATION ${CMAKE_INSTALL_CMAKEDATADIR})
install(FILES cmake/SetCompilerFlags.cmake DESTINATION ${CMAKE_INSTALL_CMAKEDATADIR})
install(FILES cmake/FindROOT.cmake DESTINATION ${CMAKE_INSTALL_CMAKEDATADIR})
install(FILES cmake/FindOpenCL.cmake DESTINATION ${CMAKE_INSTALL_CMAKEDATADIR})
install(FILES cmake/RootUseFile.cmake DESTINATION ${CMAKE_INSTALL_CMAKEDATADIR})
install(FILES ${CMAKE_BINARY_DIR}/UseBioDynaMo.cmake DESTINATION ${CMAKE_INSTALL_CMAKEDATADIR})
# CMake files required from external projects
install(FILES cmake/BioDynaMoConfig.cmake DESTINATION ${CMAKE_INSTALL_CMAKEDIR})
#simulation template
install(DIRECTORY util/simulation-template DESTINATION "biodynamo/" FILES_MATCHING PATTERN "*")
# Demos.
install(DIRECTORY demo DESTINATION "biodynamo/" PATTERN "build/" EXCLUDE)

if(LINUX)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/paraview-plugin/libBDMGlyphFilter.so DESTINATION ${CMAKE_INSTALL_PVPLUGINDIR})
elseif(APPLE)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/paraview-plugin/libBDMGlyphFilter.dylib DESTINATION ${CMAKE_INSTALL_PVPLUGINDIR})
endif()

install(FILES LICENSE NOTICE DESTINATION ${CMAKE_INSTALL_ROOT})
