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
file(GLOB_RECURSE GUI_FILES_CC
  "${BioDynaMo_SOURCE_DIR}/src/gui/*.cc"
)

file(GLOB_RECURSE GUI_FILES_H
  "${BioDynaMo_SOURCE_DIR}/src/gui/*.h"
)

set(GUI_BIN_NAME "gui")

bdm_add_executable("${GUI_BIN_NAME}"
                   SOURCES "${GUI_FILES_CC}"
                   HEADERS "${GUI_FILES_H}"
                   LIBRARIES biodynamo)

set(GUI_ICONS ${BioDynaMo_SOURCE_DIR}/src/gui/icons)