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

# This file contains functions required to build BioDynaMo

# function get_implicit_dependencies( RET_VAR
#                                     SOURCES source1 source2 ...)
# Returns a list of biodynamo headers that are included from a list of source
# files.
# \param RET_VAR variable in which the list of headers will be stored
# \param SOURCES list of source files that should be analyzed
function(get_implicit_dependencies RET_VAR)
  cmake_parse_arguments(ARG "" "" "SOURCES" ${ARGN} )

  # build include path string
  get_directory_property(INCLUDE_DIRS INCLUDE_DIRECTORIES)
  set(INCLUDE_OPTIONS)
  foreach( DIR ${INCLUDE_DIRS})
    set(INCLUDE_OPTIONS "${INCLUDE_OPTIONS} -I${DIR}")
  endforeach()

  # grep pattern to extract bdm headers
  set(BDM_HEADER_PATTERN "^${PROJECT_SOURCE_DIR}/src/|^${PROJECT_SOURCE_DIR}/test/|^${PROJECT_SOURCE_DIR}/demo/|^src/|^test/|^demo/")

  # execute shell script and store result into variable
  execute_process(
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMAND cmake/get_bdm_includes.sh ${CMAKE_CXX_COMPILER} "${INCLUDE_OPTIONS}" ${BDM_HEADER_PATTERN} ${ARG_SOURCES}
    OUTPUT_VARIABLE CMD_RESULT)

  # return result
  set(${RET_VAR} ${CMD_RESULT} PARENT_SCOPE)
endfunction(get_implicit_dependencies)

find_program(GENREFLEX_EXECUTABLE genreflex HINTS $ENV{ROOTSYS}/bin)

# function bdm_generate_dictionary (TARGET BINARY
#                                   EXECUTABLE
#                                   DICT dictionary
#                                   DEPENDS
#                                   SELECTION selectionfile ...
#                                   HEADERS header1 header2 ...
#                                   DEPENDS target1 target2
#                                   OPTIONS opt1...)
# BDM version of ROOT's genreflex macro. Uses add_custom_target instead of
# add_custom_command. This change is required to add dependencies between the
# object library dictionary generation and creation of the final shared lib/
# executable.
# Ouput of genreflex is piped into a log file ${TARGET}.log
function(bdm_generate_dictionary TARGET EXECUTABLE)
  CMAKE_PARSE_ARGUMENTS(ARG "" "DICT" "SELECTION;HEADERS;DEPENDS;OPTIONS" "" ${ARGN})
  #---Get the list of header files-------------------------
  set(headerfiles)
  foreach(fp ${ARG_HEADERS})
    file(GLOB files ${fp})
    if(files)
      foreach(f ${files})
        set(headerfiles ${headerfiles} ${f})
      endforeach()
    else()
      set(headerfiles ${headerfiles} ${fp})
    endif()
  endforeach()
  #---Get Selection file------------------------------------
  if(IS_ABSOLUTE ${ARG_SELECTION})
    set(selectionfile ${ARG_SELECTION})
  else()
    set(selectionfile ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_SELECTION})
  endif()
  #---Get the list of include directories------------------
  get_directory_property(incdirs INCLUDE_DIRECTORIES)
  set(includedirs)
  foreach( d ${incdirs})
    set(includedirs ${includedirs} -I${d})
  endforeach()
  #---Get preprocessor definitions--------------------------
  get_directory_property(defs COMPILE_DEFINITIONS)
  foreach( d ${defs})
    # definitions that were initialily defined with escaped quotes
    # e.g. add_definitions(-DFOO=\"bar\") are changed to normal quotes in ${d}
    # The string replace call has been added to fix this issue
    string(REPLACE "\"" "\\\"" d_fixed ${d})
    set(definitions ${definitions} -D${d_fixed})
  endforeach()
  #---Actual command----------------------------------------
  file(WRITE ${ARG_DICT} "")
  # determine when dictionary should be rebuilt
  # solves problem that add_custom_command does not have a target name that
  # can be used in add_dependencies and add_custom_target is always executed
  # if CMake configuration changes always rebuild dictionary -> remove file
  file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/rebuild_${TARGET}.txt)
  add_custom_command(
    OUTPUT rebuild_${TARGET}.txt
    COMMAND echo 1 >rebuild_${TARGET}.txt
    DEPENDS ${headerfiles}
    COMMENT "Build dictionary ${TARGET}")

  SET(DICT_PCM_NAME ${ARG_DICT})
  STRING(REPLACE ${CMAKE_BINARY_DIR} "" DICT_PCM_NAME ${DICT_PCM_NAME})
  STRING(REPLACE ".cc" "" DICT_PCM_NAME ${DICT_PCM_NAME})
  STRING(REPLACE "/" "" DICT_PCM_NAME ${DICT_PCM_NAME})

  # During installation we need launcher.sh to build dictionaries (since we
  # do not source environmentals). AFter the installation we do not need the
  # wrapper script, since we expect environmentals to be set.
  set(LAUNCHER)
  if(NOT DEFINED "$ENV{BDM_INSTALL_DIR}" AND NOT DEFINED ENV{BDM_INSTALL_DIR})
    set(LAUNCHER ${CMAKE_BINARY_DIR}/launcher.sh)
  endif()

  # invoke genreflex only if rebuild_${TARGET}.txt file does not contain a 0.
  # Had issues with if [[ ]] statement; used grep instead
  # if grep does not find the pattern it has a non zero exit code
  # --> grep 0 file || commandz`
  #   command is executed if pattern 0 is not found in file
  add_custom_target(${TARGET}
    COMMAND grep 0 ${CMAKE_CURRENT_BINARY_DIR}/rebuild_${TARGET}.txt >/dev/null || ${LAUNCHER}
    ${GENREFLEX_EXECUTABLE} ${headerfiles} -o ${ARG_DICT} ${rootmapopts} --select=${selectionfile}
    ${ARG_OPTIONS} ${includedirs} ${definitions} -v >${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.log 2>&1
    COMMAND echo 0 > ${CMAKE_CURRENT_BINARY_DIR}/rebuild_${TARGET}.txt
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  DEPENDS ${ARG_DEPENDS} ${CMAKE_CURRENT_BINARY_DIR}/rebuild_${TARGET}.txt)

  if (${EXECUTABLE} AND DEFINED CMAKE_INSTALL_BINDIR)
    add_custom_command(TARGET ${TARGET}
            POST_BUILD
            COMMAND cp ${CMAKE_BINARY_DIR}/${DICT_PCM_NAME}_rdict.pcm ${CMAKE_INSTALL_BINDIR})
  endif()
  # TODO after ROOT has been updated to new ROOT only copy to lib dir
  # if EXECUTABLE is false
  if (DEFINED CMAKE_INSTALL_LIBDIR)
    add_custom_command(TARGET ${TARGET}
            POST_BUILD
            COMMAND cp ${CMAKE_BINARY_DIR}/${DICT_PCM_NAME}_rdict.pcm ${CMAKE_INSTALL_LIBDIR})
  endif()

endfunction(bdm_generate_dictionary)


# function bdm_add_executable( TARGET
#                              SOURCES source1 source2 ...
#                              HEADERS header1 header2 ...
#                              LIBRARIES lib1 lib2 ...)
# BioDynaMo's version of add_executable
# This is required because ROOT dictionaries must be built beforehand.
# To make debugging of compile errors easier an object library with SOURCES
# is compiled first, then dictionaries are generated. Afterwards these
# dictionaries are compiled and linked with the object files compiled in the
# first step.
# Uses the variable `BDM_CMAKE_DIR` to point to the cmake directory. This is
# necessary, since this function is used from within the BioDynaMo repository
# and external simulation projects.
# \param TARGET target name for the executable
# \param SOURCES list of source files
# \param LIBRARIES list of libraries that should be linked to the executable.
#        can also be a target name of a library
function(bdm_add_executable TARGET)
  cmake_parse_arguments(ARG "" "" "SOURCES;HEADERS;LIBRARIES" ${ARGN} )

  if(dict)
    add_library(${TARGET}-objectlib OBJECT ${ARG_SOURCES})

    # generate dictionary using genreflex
    set(DICT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_dict.cc")
    bdm_generate_dictionary(${TARGET}-dict ON
      DICT "${DICT_FILE}"
      HEADERS ${ARG_HEADERS}
      SELECTION $ENV{BDM_CMAKE_DIR}/selection.xml
      DEPENDS ${TARGET}-objectlib)

    # generate executable
    add_executable(${TARGET} $<TARGET_OBJECTS:${TARGET}-objectlib> ${DICT_FILE})
    add_dependencies(${TARGET} ${TARGET}-dict)
    if (OPENCL_FOUND)
      # Do this here; we don't want libbiodynamo.so to contain any OpenCL symbols
      set(ARG_LIBRARIES ${ARG_LIBRARIES} ${OPENCL_LIBRARIES})
      target_compile_definitions(${TARGET}-objectlib PUBLIC -DUSE_OPENCL)
      target_compile_definitions(${TARGET} PUBLIC -DUSE_OPENCL)
    endif()
    target_link_libraries(${TARGET} ${ARG_LIBRARIES})
  else()
    add_executable(${TARGET} ${ARG_SOURCES})
    target_link_libraries(${TARGET} ${ARG_LIBRARIES})
  endif()
endfunction(bdm_add_executable)

# function build_shared_library( TARGET
#                              SOURCES source1 source2 ...
#                              HEADERS header1 header2 ...
#                              LIBRARIES lib1 lib2 ...
#                              PLUGIN <"TRUE">)
# build shared library with ROOT dictionaries. If ARG_PLUGIN is set, we will
# always generate dictionaries (as required by ROOT's plugin manager)
function(build_shared_library TARGET)
  cmake_parse_arguments(ARG "" "" "SOURCES;HEADERS;LIBRARIES;PLUGIN" ${ARGN} )

  # We always need dictionaries for the plugins (plugin manager requires class
  # information)
  if(dict OR DEFINED ARG_PLUGIN)
    add_library(${TARGET}-objectlib OBJECT ${ARG_SOURCES})

    # generate dictionary using genreflex
    set(DICT_FILE "${CMAKE_CURRENT_BINARY_DIR}/lib${TARGET}_dict.cc")
    bdm_generate_dictionary(${TARGET}-dict OFF
      DICT "${DICT_FILE}"
      HEADERS ${ARG_HEADERS}
      SELECTION $ENV{BDM_CMAKE_DIR}/selection-lib${TARGET}.xml
      DEPENDS ${TARGET}-objectlib)

    # generate shared library
    add_library(${TARGET} SHARED $<TARGET_OBJECTS:${TARGET}-objectlib> ${DICT_FILE})
    add_dependencies(${TARGET} ${TARGET}-dict update-version-info)
    target_link_libraries(${TARGET} ${ARG_LIBRARIES})

    SET(BIODYNAMO_TARGET_NAME "${TARGET}-objectlib" PARENT_SCOPE)
  else()
    add_library(${TARGET} SHARED ${ARG_SOURCES})
    add_dependencies(${TARGET} update-version-info)
    target_link_libraries(${TARGET} ${ARG_LIBRARIES})

    SET(BIODYNAMO_TARGET_NAME "${TARGET}" PARENT_SCOPE)
  endif()
endfunction(build_shared_library)

# function generate_rootlogon
# generates rootlogon.C which is required by ROOT's C++ interpreter cling
function(generate_rootlogon)
  set(CONTENT "{")
  get_directory_property(INCLUDE_DIRS INCLUDE_DIRECTORIES)
  set(INCLUDE_OPTIONS)
  
  # if USE_DICT is set for libbiodynamo.so, we also need to set it for rootcling
  # when we want to use the interpreter or notebooks. Otherwise there would break
  # the one-definition rule
  if (dict)
    set(CONTENT "${CONTENT}\n  gROOT->ProcessLine(\"#define USE_DICT\")\;")
  endif()

  set(CONTENT "${CONTENT}\n  gROOT->ProcessLine(\"R__LOAD_LIBRARY(libbiodynamo)\")\;")
  # We add this one because the ROOT visualization require it, and it's not one
  # of the core libraries that is loaded by default in rootcling
  set(CONTENT "${CONTENT}\n  gROOT->ProcessLine(\"R__LOAD_LIBRARY(GenVector)\")\;")
  set(CONTENT "${CONTENT}\n  gROOT->ProcessLine(\"#include \\\"biodynamo.h\\\"\")\;")
  set(CONTENT "${CONTENT}\n  gROOT->ProcessLine(\"using namespace bdm\;\")\;")
  set(CONTENT "${CONTENT}\n}\n")

  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/rootlogon.C" ${CONTENT})

  # If no .rootrc exists in the home directory, then we make one for the
  # convenience of not having to load our rootlogon.C explicitely for every
  # cling session.
  set(ROOTRC_PATH "$ENV{HOME}/.rootrc")
  set(ROOTRC_CONTENT "Unix.*.Root.MacroPath:      .:$(ROOTSYS)/macros:$(BDMSYS)/etc\n")
  if(NOT EXISTS "${ROOTRC_PATH}")
    file(WRITE "${ROOTRC_PATH}" "${ROOTRC_CONTENT}")
  endif()
endfunction(generate_rootlogon)

# generates a target to build the biodynamo paraview plugin
function(build_paraview_plugin)
  set(PV_PLUGIN_BINDIR ${CMAKE_CURRENT_BINARY_DIR}/paraview-plugin)
  file(MAKE_DIRECTORY ${PV_PLUGIN_BINDIR})

  if(LINUX)
      SET(OUTPUT_PLUGIN_NAME libBDMGlyphFilter.so)
  elseif(APPLE)
      SET(OUTPUT_PLUGIN_NAME libBDMGlyphFilter.dylib)
  endif()

  add_custom_target(paraview-plugin
    ALL
    COMMAND cmake ../../paraview_plugin/bdm_glyph && cmake --build . --target all
    COMMAND cmake -E make_directory ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/pv_plugin
    COMMAND cp ${OUTPUT_PLUGIN_NAME} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/pv_plugin
    WORKING_DIRECTORY ${PV_PLUGIN_BINDIR}
    COMMENT "Build bdm paraview plugin")


endfunction(build_paraview_plugin)

# Fix for rootcling not able to find omp.h. We cannot include the entire include
# directory of the compiler (where omp.h is in), because rootcling starts using
# the intrinsics found in there. Therefore we do a local copy into the build dir.
function(fix_rootcling_omp_issue)

  execute_process(COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=include/omp.h
                  OUTPUT_VARIABLE OMP_HEADER_PATH)
  # above command returns path with "\n" appended
  string(REGEX REPLACE "\n$" "" OMP_HEADER_PATH "${OMP_HEADER_PATH}")
  if ("${OMP_HEADER_PATH}" STREQUAL "include/omp.h")
     execute_process(COMMAND cp -a ${CMAKE_SOURCE_DIR}/omp ${CMAKE_BINARY_DIR})
  else()
    execute_process(COMMAND mkdir -p ${CMAKE_BINARY_DIR}/omp)
    execute_process(COMMAND cp ${OMP_HEADER_PATH} ${CMAKE_BINARY_DIR}/omp)
  endif()
  include_directories("${CMAKE_BINARY_DIR}/omp")

endfunction(fix_rootcling_omp_issue)
