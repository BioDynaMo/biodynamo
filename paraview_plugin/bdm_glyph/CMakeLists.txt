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

SET(BDMGLYPH_SOURCES ${CMAKE_SOURCE_DIR}/paraview_plugin/bdm_glyph/BDMGlyphFilter.cc
        ${CMAKE_SOURCE_DIR}/paraview_plugin/bdm_glyph/BDMGlyph.cc)

include_directories(${CMAKE_SOURCE_DIR}/paraview_plugin/bdm_glyph/)

# Use the ADD_PARAVIEW_PLUGIN macro to build a plugin
ADD_PARAVIEW_PLUGIN(
  BDMGlyphFilter              #<--Name for the plugin
 "1.0"                       #<--Version string
  SERVER_MANAGER_XML ${CMAKE_SOURCE_DIR}/paraview_plugin/bdm_glyph/bdm_glyph_filter.xml  #<-- server manager xml
  SERVER_MANAGER_SOURCES ${BDMGLYPH_SOURCES} #<-- source files for the new classes
)