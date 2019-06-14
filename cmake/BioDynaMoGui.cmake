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

find_program(PYTHON_EXECUTABLE python)

if(NOT PYTHON_EXECUTABLE)
		MESSAGE(FATAL_ERROR "Python not found! Aborting...")
endif()

set(GUI_DICT)
execute_process(COMMAND ${PYTHON_EXECUTABLE} ${BioDynaMo_SOURCE_DIR}/src/gui/generate_dict.py
                OUTPUT_VARIABLE GUI_DICT)

include_directories(${BioDynaMo_SOURCE_DIR}/src/gui/*)

file(GLOB_RECURSE CC_FILES
  "${BioDynaMo_SOURCE_DIR}/src/gui/*.cc",
  "${BioDynaMo_SOURCE_DIR}/src/core/util/io.cc"
)

ADD_LIBRARY(LIBS 
    ${CC_FILES}
)

target_link_libraries(LIBS ${ROOT_LIBRARIES})

file(GLOB sourcefiles ${BioDynaMo_SOURCE_DIR}/src/gui/*)

add_executable(gui ${sourcefiles})

target_link_libraries(gui LIBS)

set(GUI_ICONS ${BioDynaMo_SOURCE_DIR}/src/gui/icons)