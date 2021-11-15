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

# Generate notebook from demo NB_NAME
function(GenerateNotebookTarget NB_NAME)
  set(ENV{PYTHONPATH} "${ROOTSYS}/lib")
  if(NOT BDM_OUT_OF_SOURCE)
    set(LAUNCHER ${CMAKE_BINARY_DIR}/launcher.sh)
  endif()
  add_custom_target(notebook-${NB_NAME}
      COMMAND bash -c "${LAUNCHER} jupyter nbconvert --to=html --ExecutePreprocessor.timeout=180 --execute ${CMAKE_BINARY_DIR}/notebook/${NB_NAME}.ipynb")
  add_dependencies(notebook-${NB_NAME} biodynamo)
  add_dependencies(notebook-${NB_NAME} copy_files_bdm)
endfunction(GenerateNotebookTarget)

if(notebooks)
  add_custom_target(notebooks ALL COMMENT "Execute BioDynaMo Notebooks and generate HTML output")
  add_dependencies(notebooks biodynamo)
  add_dependencies(notebooks copy_files_bdm)
  file(GLOB NOTEBOOKS_PATHS LIST_DIRECTORIES false CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/notebook/*.ipynb")
  set(NOTEBOOKS "")
  foreach(FILE ${NOTEBOOKS_PATHS})
    get_filename_component(NB_NAME ${FILE} NAME_WE)
    list(APPEND NOTEBOOKS ${NB_NAME})
  endforeach()
  message("NOTEBOOKS = ${NOTEBOOKS}")
  foreach(NB_NAME ${NOTEBOOKS})
    GenerateNotebookTarget(${NB_NAME})
    add_dependencies(notebooks notebook-${NB_NAME})
  endforeach()
endif()
