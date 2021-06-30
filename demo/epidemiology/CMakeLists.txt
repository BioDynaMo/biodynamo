# -----------------------------------------------------------------------------
#
# Copyright (C) Lukas Breitwieser.
# All Rights Reserved.
#
# ----------------------------------------------------------------------------- 

cmake_minimum_required(VERSION 3.2.0)

project(epidemiology)

include(ExternalProject)
ExternalProject_Add(
  optim
  URL file:///${CMAKE_SOURCE_DIR}/optim-18ae71.zip
  # BUILD_IN_SOURCE 1
  BINARY_DIR "optim-prefix/src/optim"
  # CONFIGURE_COMMAND "pwd"
  CONFIGURE_COMMAND ./configure
  # CONFIGURE_COMMAND ${cmd}
  # CONFIGURE_COMMAND ./configure --header-only-version
  # BUILD_COMMAND ""
  INSTALL_COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/optim-prefix/src/optim/${CMAKE_SHARED_LIBRARY_PREFIX}optim.so ${CMAKE_CURRENT_BINARY_DIR}
  )

add_library(liboptim SHARED IMPORTED)
set_property(TARGET liboptim PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}optim.so)
add_dependencies(liboptim optim)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/optim-prefix/src/optim/include)
link_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(BioDynaMo REQUIRED)
include(${BDM_USE_FILE})
include_directories("src")

find_package(GSL REQUIRED)
include_directories(${GSL_INCLUDE_DIR})

file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE SOURCES src/*.cc)

bdm_add_executable(epidemiology
                   HEADERS ${HEADERS}
                   SOURCES ${SOURCES}
                   LIBRARIES ${BDM_REQUIRED_LIBRARIES} GSL::gsl GSL::gslcblas liboptim)

