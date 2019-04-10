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
# Locate ParaView build and then import CMake configuration,
# macros etc. from it.
FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})
link_directories(${ParaView_DIR}/../..)

find_package(ospray 1.8.0 REQUIRED)
if(NOT ${OSPRAY_FOUND})
  message(ERROR "OSPRAY not found")
endif()

# Use the ADD_PARAVIEW_PLUGIN macro to build a plugin
ADD_PARAVIEW_PLUGIN(
  BDMGlyphFilter              #<--Name for the plugin
  "1.0"                       #<--Version string
  SERVER_MANAGER_XML bdm_glyph_filter.xml  #<-- server manager xml
  SERVER_MANAGER_SOURCES BDMGlyphFilter.cc BDMGlyph.cc #<-- source files for the new classes
)
