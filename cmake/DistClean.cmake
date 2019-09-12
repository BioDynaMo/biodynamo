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

# All the files we want to remove. We keep Makefile and CMakeFiles.
SET(ALL_FILES *.txt *.sh *.cxx *.h *.data *.args *.log *.cc *.xml *.pcm *.xsl *.json *.C *.cmake *.root Doxyfile
    CMakeDoxyfile.in)
SET(ALL_DIRECTORIES extracted-third-party-libs gtest omp version output result-dir Testing)

# Remove all the files
add_custom_target(remove-files
COMMAND rm -f ${ALL_FILES}
WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# Remove all the directories
add_custom_target(remove-dirs
COMMAND rm -rf ${ALL_DIRECTORIES}
WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# Remove everything
add_custom_target(distclean
DEPENDS remove-files remove-dirs)
