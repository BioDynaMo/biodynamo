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

# add a target to generate API documentation with Doxygen


# "Calling Doxygen from cmake" from Thomas Pircher
# used under a Creative Commons Attribution-Share Alike 3.0 Unported License
# https://creativecommons.org/licenses/by-sa/3.0/
# Changes where made; original article can be found at:
# https://www.tty1.net/blog/2014/cmake-doxygen_en.html
function(GenerateAPIDocTarget)
  # create output directory
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doc")

  set(doxyfile_in "${CMAKE_CURRENT_SOURCE_DIR}/doc/api/Doxyfile.in")
  set(doxyfile "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")

  configure_file(${doxyfile_in} ${doxyfile} @ONLY)
  set(DEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/doc/api)
  add_custom_target(bdm_doc
      OUTPUT "${DEST_DIR}/index.html" 
      DEPENDS biodynamo ${doxyfile}
      COMMAND rm -rf ${DEST_DIR}
      COMMAND mkdir -p ${DEST_DIR}
      COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
      COMMENT "Generate documentation"
      VERBATIM)

  add_custom_target(doc DEPENDS "${DEST_DIR}/index.html")

  # Issue with long file names when building deb pacakages
  # https://gitlab.kitware.com/cmake/cmake/issues/14332
  # Issue resolved in CMake 3.7.2
  # install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doc/html" DESTINATION share/doc)
endfunction(GenerateAPIDocTarget)

# ------------------------------------------------------------------------------
find_package(Doxygen)

if(DOXYGEN_FOUND)

  # Check if the dot component was found
  IF (NOT DOXYGEN_DOT_FOUND)
    MESSAGE(WARNING "The dot (graphviz) component form Doxygen was not found. This will reduce \
the functionalities of Doxygen and it will prevent it from generating some graphs and \
visualizations in the documentation. Please install dot (graphviz) by issuing './prerequisites.sh all'.")
  ENDIF()

  GenerateAPIDocTarget()
else()
  message(WARNING "Could not find Doxygen. Target doc won't be available. Therefore you will not \
be able to build BioDynaMo's documentation.")
endif()
