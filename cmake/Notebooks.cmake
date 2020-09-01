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

# Adds a target to generate JupyROOT notebooks (interactive mode and static
# html) and ROOT macros from the available notebooks

# Macro to get all subdirectories from a directory
MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

# Generate notebook from demo DEMO_NAME
function(GenerateNotebookTarget DEMO_NAME)
  set(DEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/notebooks/${DEMO_NAME})
  set(DEMO_DIR ${PROJECT_SOURCE_DIR}/demo/${DEMO_NAME})
  set(SCRIPT   ${PROJECT_SOURCE_DIR}/util/demo_to_notebook.py)
  set(ENV{PYTHONPATH} "${ROOTSYS}/lib")
  set(LAUNCHER ${CMAKE_BINARY_DIR}/launcher.sh)
  add_custom_target(notebook-${DEMO_NAME}
      COMMAND rm -rf ${DEST_DIR}
      COMMAND mkdir -p ${DEST_DIR}
      COMMAND cp ${DEMO_DIR}/src/*.h ${DEST_DIR}
      COMMAND cp ${DEMO_DIR}/thumbnail.* ${DEST_DIR}
      COMMAND bash -c "${LAUNCHER} python -E ${SCRIPT} --tutpath=${DEMO_DIR}/src/${DEMO_NAME}.h --outdir=${DEST_DIR}")
  add_dependencies(notebook-${DEMO_NAME} biodynamo)
endfunction(GenerateNotebookTarget)

# Demos to skip over
set(SKIPLIST "soma_clustering" "tumor_concept" "multiple_simulations" "gene_regulation" "sbml_integration" "makefile_project")

# We chain the targets of the demos to each other, because of a race conditions
# that occurs when invoking jupyter-notebook with multiple processes:
# ".ipython/metakernel/history already exists"
if(notebooks)
  add_custom_target(notebooks ALL COMMENT "Generate notebooks")
  add_dependencies(notebooks biodynamo)
  SUBDIRLIST(DEMOS ${PROJECT_SOURCE_DIR}/demo)
  foreach(DEMO ${DEMOS})
    if(${DEMO} IN_LIST SKIPLIST)
      list(REMOVE_ITEM DEMOS ${DEMO})
    endif()
  endforeach()
  list(LENGTH DEMOS NUM_DEMOS)
  math(EXPR NUM_DEMOS "${NUM_DEMOS} - 1")
  foreach(IT RANGE ${NUM_DEMOS})
    list(GET DEMOS ${IT} DEMO)
    GenerateNotebookTarget(${DEMO})
    if(IT EQUAL 0)
      add_dependencies(notebooks notebook-${DEMO})
    else()
      math(EXPR ITD "${IT} - 1")
      list(GET DEMOS ${ITD} PREVIOUS_DEMO)
      add_dependencies(notebook-${PREVIOUS_DEMO} notebook-${DEMO})
    endif()
  endforeach()
endif()
