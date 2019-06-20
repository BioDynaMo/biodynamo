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

# Options. Turn on with 'cmake -Dmyvarname=ON'.
option(cuda      "Enable CUDA code generation for GPU acceleration" OFF)
option(opencl    "Enable OpenCL code generation for GPU acceleration" OFF)
option(dict      "Build with ROOT dictionaries" ON)
option(paraview  "Enable ParaView" ON)

# This file contains the build setup for simulation projects outside the
# biodynamo repository
# Usage:
#   find_package(BioDynaMo REQUIRED)
#   include(${BDM_USE_FILE})
#   bdm_add_executable(...)

# Add our CMake files (e.g. FindXXX.cmake files) to the module path, so that out
# of source build can find them
get_filename_component(CMAKE_DIR ${BDM_USE_FILE} DIRECTORY)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_DIR})

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

if(DEFINED ENV{BDM_CMAKE_DIR})
    set(BDM_CMAKE_DIR $ENV{BDM_CMAKE_DIR})
    add_definitions(-DBDM_SRC_DIR=\"$ENV{BDM_SRC_DIR}\")
else()
  execute_process(COMMAND rm -rf ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles)
  message(FATAL_ERROR "The BioDynaMo environment is not set up correctly. Please call 'source <path-to-bdm-installation>/biodynamo-env.sh' and retry this command.")
endif()

# -------------------- find packages ------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "$ENV{BDM_INSTALL_DIR}/biodynamo/cmake/")
# CMake cannot find LLVM's Clang OpenMP library by default (this makes
# find_package(OpenMP) fail). Therefore we manually specify the OpenMP flags.
# Furthermore, rootcling cannot find omp.h by default, so we copy this (with a
# shell script to /usr/local/Cellar/biodynamo, which is brew's default install
# directory), and include this directory. We cannot directly include the
# original directory, because of header conflicts (such as stdint.h)
if(APPLE)
  link_directories(/usr/local/opt/llvm/lib)
  set(OpenMP_C_FLAGS -fopenmp=libomp)
  set(OpenMP_CXX_FLAGS -fopenmp=libomp)
else()
  find_package(OpenMP REQUIRED)
endif()
find_package(Git)
if(cuda)
  find_package(CUDA)
endif()
if(opencl)
  find_package(OpenCL)
endif()

find_package(Numa REQUIRED)
if (NUMA_FOUND)
   include_directories(${NUMA_INCLUDE_DIR})
   link_directories(${NUMA_LIBRARY_DIR})
   add_definitions("-DUSE_NUMA")
   set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${NUMA_LIBRARIES})
endif()

if(paraview)
  find_package(ParaView 5.3 QUIET OPTIONAL_COMPONENTS vtkPVPythonCatalyst)
  if(${ParaView_FOUND})
    message(STATUS "Found ParaView")
    include("${PARAVIEW_USE_FILE}")
    add_definitions("-DUSE_CATALYST")
    link_directories($ENV{ParaView_LIB_DIR})
    set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} vtkPVPythonCatalyst
        vtkIOParallelXML vtkPVVTKExtensionsDefault vtkPVServerManagerRendering)
  else()
    message(WARNING "ParaView not found")
    set(paraview OFF CACHE BOOL)
  endif()
endif()

# paraview version 5.5 (MacOS) does not contain TBB
if (APPLE OR (NOT paraview OR NOT ${ParaView_FOUND}))
  find_package(TBB REQUIRED)
  if (TBB_FOUND)
    include_directories(${TBB_INCLUDE_DIRS})
    link_directories(${TBB_LIBRARIES})
    set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${TBB_LIBRARIES})
  endif()
endif()

find_package(ROOT REQUIRED COMPONENTS Geom Gui)
if (dict)
  add_definitions("-DUSE_DICT")
endif()

find_package(VTune)
if(${VTune_FOUND})
  include_directories(${VTUNE_INCLUDE_DIR})
  add_definitions("-DUSE_VTUNE")
  set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${VTUNE_LIBRARIES})
else()
  message(WARNING "VTune not found")
endif()

# Link to OpenCL
if (OPENCL_FOUND)
  if (OPENCL_HAS_CXX)
    add_definitions("-DUSE_OPENCL")
    set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${OPENCL_LIBRARIES})
  else()
    message(WARNING "OpenCL C++ bindings not found. Please install to make use of OpenCL. "
      "If you think you have installed the C++ bindings correctly, please check if one "
      "of the following environmentals is set correctly (vendor specific):
      - AMD: \t\tAMDAPPSDKROOT
      - NVIDIA: \tCUDA_PATH
      - INTEL: \tINTELOPENCLSDK")
    set(OPENCL_FOUND FALSE)
  endif()
endif()

if (CUDA_FOUND)
  add_definitions("-DUSE_CUDA")
  include_directories(${CUDA_INCLUDE_DIRS} ${CUDA_TOOLKIT_ROOT_DIR})
  set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${CUDA_LIBRARIES})
  set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} bdmcuda)
endif()

# -------------------- set default build type and compiler flags ---------------
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
include("${BDM_CMAKE_DIR}/SetCompilerFlags.cmake")

# -------------------- set rpath options ---------------------------------------
# When building, use the RPATH
set(CMAKE_SKIP_BUILD_RPATH FALSE)           # don't skip the full RPATH for the build tree
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
set(CMAKE_INSTALL_RPATH "")

# -------------------- includes -----------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${BDM_CMAKE_DIR}")
include("${BDM_CMAKE_DIR}/BioDynaMo.cmake")
include(${ROOT_USE_FILE})

fix_rootcling_omp_issue()

set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} biodynamo ${ROOT_LIBRARIES})

include_directories("$ENV{BDM_CMAKE_DIR}/../../include")
link_directories("$ENV{BDM_CMAKE_DIR}/../../lib")
