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

find_package(PkgConfig)
if (PkgConfig_FOUND)
    pkg_check_modules(LIBGIT2 libgit2)
endif ()

if (LIBGIT2_FOUND)
    message(STATUS "Found libgit2: ${LIBGIT2_LIBRARIES}")
    # include directories
    include_directories(${LIBGIT2_INCLUDE_DIRS})
    # Add to required libraries
    set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${LIBGIT2_LIBRARIES})
    # Make linker aware of the library path
    link_directories(${LIBGIT2_LIBRARY_DIRS})
    # Define the USE_LIBGIT2 flag
    add_definitions(-DUSE_LIBGIT2)
else()
    message(WARNING "Libgit2 not found. GitTracking will not be available.")
endif()
