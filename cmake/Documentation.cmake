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
  add_custom_target(doc-api
      COMMAND rm -rf ${DEST_DIR}
      COMMAND mkdir -p ${DEST_DIR}
      COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
      COMMENT "Generating API documentation with Doxygen.
       Open the following file in your browser: ${DEST_DIR}/index.html"
      VERBATIM)

  # Issue with long file names when building deb pacakages
  # https://gitlab.kitware.com/cmake/cmake/issues/14332
  # Issue resolved in CMake 3.7.2
  # install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doc/html" DESTINATION share/doc)
endfunction(GenerateAPIDocTarget)

# Generate mkdocs documentation
# DOC_TYPE is either user or dev
function(GenerateMkDocsTarget DOC_TYPE)
  set(DEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/doc/${DOC_TYPE})
  set(DOC_DIR ${PROJECT_SOURCE_DIR}/doc/${DOC_TYPE}_guide)
  add_custom_target(doc-${DOC_TYPE}
      COMMAND rm -rf ${DEST_DIR}
      COMMAND mkdir -p ${DEST_DIR}
      COMMAND bash -c "${MKDOCS_PROGRAM} build"
      COMMAND bash -c "mv site/* ${DEST_DIR}"
      WORKING_DIRECTORY "${DOC_DIR}"
      COMMENT "Generating ${DOC_TYPE} documentation with mkdocs.
       Open the following file in your browser: ${DEST_DIR}/index.html"
      VERBATIM)
endfunction(GenerateMkDocsTarget)


function(GenerateLiveMkDocsTarget DOC_TYPE)
  set(DOC_DIR ${PROJECT_SOURCE_DIR}/doc/${DOC_TYPE}_guide)
  add_custom_target(live-${DOC_TYPE}-guide
      COMMAND bash -c "${MKDOCS_PROGRAM} serve"
      WORKING_DIRECTORY "${DOC_DIR}"
      VERBATIM)
endfunction(GenerateLiveMkDocsTarget)

# ------------------------------------------------------------------------------
find_package(Doxygen)
find_package(MKDocs)

if(DOXYGEN_FOUND AND MKDocs_FOUND)
  GenerateAPIDocTarget()
  GenerateMkDocsTarget(user)
  GenerateMkDocsTarget(dev)

  GenerateLiveMkDocsTarget(user)
  GenerateLiveMkDocsTarget(dev)

  add_custom_target(doc COMMENT "Generate documentation")
  add_dependencies(doc doc-api doc-user doc-dev)
else()
  message(WARNING "Could not find Doxygen or mkdocs. Target doc won't be available.")
endif()
