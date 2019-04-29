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
include(FindROOT)
include(${ROOT_USE_FILE})

generate_rootlogon()

include_directories(${BioDynaMo_SOURCE_DIR}/src/gui/*)
add_executable(gui ${BioDynaMo_SOURCE_DIR}/src/gui/gui.cc)

target_link_libraries(gui ${ROOT_LIBRARIES})

MESSAGE("ROOT_LIBRARIES=")
MESSAGE(${ROOT_LIBRARIES})
