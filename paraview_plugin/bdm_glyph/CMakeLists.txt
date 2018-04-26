cmake_minimum_required(VERSION 3.3)
project(BDMGlyph)
# Locate ParaView build and then import CMake configuration,
# macros etc. from it.
FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})

# Use the ADD_PARAVIEW_PLUGIN macro to build a plugin
ADD_PARAVIEW_PLUGIN(
  BDMGlyphFilter              #<--Name for the plugin
  "1.0"                       #<--Version string
  SERVER_MANAGER_XML bdm_glyph_filter.xml  #<-- server manager xml
  SERVER_MANAGER_SOURCES BDMGlyphFilter.cc BDMGlyph.cc #<-- source files for the new classes
)
