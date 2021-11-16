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

# add a target to generate BioDynaMo website using Gatsby static generator

function(GenerateStaticFiles TARGET FLAGS)
  set(WEB_DIR ${CMAKE_CURRENT_BINARY_DIR}/website)
  file(MAKE_DIRECTORY "${WEB_DIR}")
  add_custom_target(${TARGET}
      WORKING_DIRECTORY "${WEB_DIR}"
      COMMAND [ -f build_website.sh ] || git clone https://github.com/BioDynaMo/website.git .
      COMMAND git pull || true
      COMMENT "Generate website"
      COMMAND ./build_website.sh --dir ${CMAKE_CURRENT_SOURCE_DIR} ${FLAGS}
      VERBATIM)

  # API documentation dependency
  if (NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "You cannot build the website without Doxygen. Please make
    sure that Doxygen is installed before attempting to build the website!")
  endif()
  add_dependencies(${TARGET} doc)

  # BDM notebook dependency
  if (notebooks)
    add_dependencies(${TARGET} notebooks)
  else()
    message(FATAL_ERROR "You cannot build the website without building the BioDynaMo notebooks first.
Enable the notebook feature by adding the following cmake parameter: -Dnotebooks=ON
")
  endif()
endfunction()

if (website)
  # Generate static website files
  GenerateStaticFiles(website "--api")
  # Generate staic website files and launches website on localhost for live editing
  GenerateStaticFiles(website-live "--develop")
endif()

# ------------------------------------------------------------------------------
