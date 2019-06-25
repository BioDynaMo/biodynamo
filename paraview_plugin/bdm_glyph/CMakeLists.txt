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

cmake_minimum_required(VERSION 3.3)
project(BDMGlyph)

# Directory in which BioDynaMo will be installed. It can be provided
# by the user itself. It will also contain paraview, root and qt.
set(BDM_INSTALL_DIR ${CMAKE_BINARY_DIR}/..)

# Set Qt5 directory and find ParaView
SET(Qt5_DIR ${BDM_INSTALL_DIR}/third_party/qt/lib/cmake/Qt5)
FIND_PACKAGE(ParaView REQUIRED PATHS ${BDM_INSTALL_DIR}/third_party/paraview/lib/cmake/paraview-5.6)

# Locate ParaView build and then import CMake configuration,
# macros etc. from it.
INCLUDE(${PARAVIEW_USE_FILE})
link_directories(${ParaView_DIR}/../..)

# Use the ADD_PARAVIEW_PLUGIN macro to build a plugin
ADD_PARAVIEW_PLUGIN(
  BDMGlyphFilter              #<--Name for the plugin
  "1.0"                       #<--Version string
  SERVER_MANAGER_XML bdm_glyph_filter.xml  #<-- server manager xml
  SERVER_MANAGER_SOURCES BDMGlyphFilter.cc BDMGlyph.cc #<-- source files for the new classes
)
