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

# add a target to generate BioDynaMo website using Gatsby static generator

function(GenerateStaticFiles TARGET FLAGS)
  set(WEB_DIR ${CMAKE_CURRENT_BINARY_DIR}/website)
  file(MAKE_DIRECTORY "${WEB_DIR}")
  add_custom_target(${TARGET}
      WORKING_DIRECTORY "${WEB_DIR}"
      COMMAND rm -rf ${WEB_DIR} && mkdir ${WEB_DIR}
      COMMAND git clone https://github.com/BioDynaMo/website.git .
      # TODO: merge into master to avoid this
      COMMAND git checkout web-updates
      COMMENT "Generate website"
      COMMAND ./build_website.sh --dir ${CMAKE_CURRENT_SOURCE_DIR} ${FLAGS}
      VERBATIM)
  add_dependencies(${TARGET} doc)
endfunction()

if (website)
  if (NOT DOXYGEN_FOUND)
    message(FATAL "You cannot build the website without Doxygen. Please make
    sure that Doxygen is installed before attempting to build the website!")
  endif()
  # Generate static website files
  GenerateStaticFiles(website "--api")
  # Generate staic website files and launches website on localhost for live editing
  GenerateStaticFiles(website-live "--develop")
endif()

# For website-live the Doxygen files are not generated because we expect users
# to mostly be interested in how the markdown files will look like. There is
# also some bug in trying to show the API files if you were to enable to it
# (i.e. it doesn't resolve the path correctly). For those that want to see the
# Doxygen output, you can open the index.html in the build/doc/api directory with
# your browser

# ------------------------------------------------------------------------------
