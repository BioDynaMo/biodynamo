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

# Check if we have set the install dir of BioDynaMo
if (NOT DEFINED ENV{BDM_INSTALL_DIR})
    SET(ENV{BDM_INSTALL_DIR} "$ENV{HOME}/.bdm")
endif()

# Find ParaViews and Qt
FIND_PACKAGE(Qt5 COMPONENTS Widgets
        PATHS ${CMAKE_BINARY_DIR}/../install/third_party/qt
        $ENV{BDM_INSTALL_DIR}/third_party/qt)
FIND_PACKAGE(ParaView REQUIRED
        PATHS $ENV{BDM_INSTALL_DIR}/third_party/paraview
        ${CMAKE_BINARY_DIR}/../install/third_party/paraview)

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
