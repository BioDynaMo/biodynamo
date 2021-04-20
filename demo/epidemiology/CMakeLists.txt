# -----------------------------------------------------------------------------
#
# Copyright (C) Lukas Breitwieser.
# All Rights Reserved.
#
# ----------------------------------------------------------------------------- 

cmake_minimum_required(VERSION 3.2.0)

project(epidemiology)

find_package(BioDynaMo REQUIRED)
include(${BDM_USE_FILE})

include_directories(${BDMSYS}/include/optim)
include_directories(${BDMSYS}/include/armadillo)
link_directories(${CMAKE_CURRENT_BINARY_DIR})

# use armadillo backend
add_definitions("-DOPTIM_ENABLE_ARMA_WRAPPERS")

include_directories("src")

find_package(GSL REQUIRED)
include_directories(${GSL_INCLUDE_DIR})

file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE SOURCES src/*.cc)

bdm_add_executable(epidemiology
                   HEADERS ${HEADERS}
                   SOURCES ${SOURCES}
                   LIBRARIES ${BDM_REQUIRED_LIBRARIES} GSL::gsl GSL::gslcblas)

