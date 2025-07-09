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
# Uses the variable `BDMSYS` to point to the cmake directory. This is
# necessary, since this function is used from within the BioDynaMo repository
# and external simulation projects.
# \param TARGET target name for the executable
# \param SOURCES list of source files
# \param LIBRARIES list of *shared* libraries that should be linked to the executable.
#        can also be a target name of a library.
#        STATIC libraries not compiled with -fPIC must be linked in a separate command using
#        target_link_libraries(${TARGET} static-lib-wo-fpic)
function(bdm_add_executable TARGET)
  cmake_parse_arguments(ARG "" "" "SOURCES;HEADERS;LIBRARIES" ${ARGN} )

  if(dict)
    if (OPENCL_FOUND)
      # Do this here; we don't want libbiodynamo.so to contain any OpenCL symbols
      set(ARG_LIBRARIES ${ARG_LIBRARIES} ${OPENCL_LIBRARIES})
    endif()
    if(mpi AND MPI_FOUND)
      set(ARG_LIBRARIES ${ARG_LIBRARIES} ${MPI_mpi_LIBRARY})
      set(ARG_LIBRARIES ${ARG_LIBRARIES} ${MPI_mpi_cxx_LIBRARY})
    endif()
    build_shared_library(${TARGET}
                         SELECTION selection.xml
                         HEADERS ${ARG_HEADERS}
                         SOURCES ${ARG_SOURCES}
                         LIBRARIES ${ARG_LIBRARIES})
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${TARGET}-main.cc "int main(int argc, const char** argv);")
    add_executable(${TARGET}-bin ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${TARGET}-main.cc)
    target_link_libraries(${TARGET}-bin ${TARGET})
    if (LINUX)
      set_target_properties(${TARGET}-bin PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN")
    else()
      set_target_properties(${TARGET}-bin PROPERTIES LINK_FLAGS "-Wl,-rpath,@loader_path")
    endif()
    set_target_properties(${TARGET}-bin PROPERTIES OUTPUT_NAME ${TARGET})
  else()
    add_executable(${TARGET} ${ARG_SOURCES})
    target_link_libraries(${TARGET} ${ARG_LIBRARIES})
  endif()
endfunction(bdm_add_executable)

# function build_shared_library( TARGET
#                                SELECTION selection.xml
#                                SOURCES source1 source2 ...
#                                HEADERS header1 header2 ...
#                                LIBRARIES lib1 lib2 ...
#                                PLUGIN <"TRUE">)
# build shared library with ROOT dictionaries. If ARG_PLUGIN is set, we will
# always generate dictionaries (as required by ROOT's plugin manager)
function(build_shared_library TARGET)
  cmake_parse_arguments(ARG "" "" "SELECTION;SOURCES;HEADERS;LIBRARIES;PLUGIN" ${ARGN} )

  # We always need dictionaries for the plugins (plugin manager requires class
  # information)
  if(dict OR DEFINED ARG_PLUGIN)
    # generate dictionary using genreflex
    set(DICT_FILE "${CMAKE_CURRENT_BINARY_DIR}/lib${TARGET}_dict")
    set(BDM_DICT_FILE "${CMAKE_CURRENT_BINARY_DIR}/lib${TARGET}_bdm_dict.cc")

    # Since the location of the CMake files differ in the build and installation
    # directory, we check if BDM_CMAKE_DIR is already set (in build directory
    # case). Otherwise, set it to the installation directory
    if(NOT DEFINED BDM_CMAKE_DIR)
      set(BDM_CMAKE_DIR $ENV{BDMSYS}/share/cmake)
    endif()
    REFLEX_GENERATE_DICTIONARY(${DICT_FILE} ${ARG_HEADERS} SELECTION ${BDM_CMAKE_DIR}/${ARG_SELECTION})
    if (BDM_OUT_OF_SOURCE)
      set(BDM_DICT_BIN_PATH "$ENV{BDMSYS}/bin")
    else()
      set(BDM_DICT_BIN_PATH "${PROJECT_SOURCE_DIR}/cmake")
    endif()
    get_property(INCLUDE_DIRS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
    if (BDM_OUT_OF_SOURCE)
      set(BDM_OUT_OF_SRC_ARG "--out-of-source")
    else()
      set(BDM_OUT_OF_SRC_ARG --bdm-source ${CMAKE_SOURCE_DIR})
    endif()
    add_custom_command(OUTPUT "${BDM_DICT_FILE}"
                       COMMAND ${Python_EXECUTABLE} ${BDM_DICT_BIN_PATH}/bdm-dictionary ${BDM_OUT_OF_SRC_ARG} --output ${BDM_DICT_FILE} --include-dirs ${INCLUDE_DIRS} --headers ${ARG_HEADERS}
                       DEPENDS ${ARG_HEADERS} ${BDM_DICT_BIN_PATH}/bdm-dictionary)
    # generate shared library
    add_library(${TARGET} SHARED ${ARG_SOURCES} ${DICT_FILE}.cc ${BDM_DICT_FILE})
    if (OPENCL_FOUND)
      target_compile_definitions(${TARGET} PUBLIC -DUSE_OPENCL)
    endif()
    target_link_libraries(${TARGET} ${ARG_LIBRARIES})
    if (DEFINED CMAKE_INSTALL_LIBDIR)
      add_custom_command(TARGET ${TARGET}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${DICT_FILE}_rdict.pcm ${CMAKE_INSTALL_LIBDIR})
    endif()
  else()
    add_library(${TARGET} SHARED ${ARG_SOURCES})
    target_link_libraries(${TARGET} ${ARG_LIBRARIES})
  endif()
endfunction(build_shared_library)

# function generate_rootlogon
# generates rootlogon.C which is required by ROOT's C++ interpreter cling
function(generate_rootlogon)
  get_directory_property(INCLUDE_DIRS INCLUDE_DIRECTORIES)
  set(INCLUDE_OPTIONS)

  set(CONTENT "{")
  # if USE_DICT is set for libbiodynamo.so, we also need to set it for rootcling
  # when we want to use the interpreter or notebooks. Otherwise there would break
  # the one-definition rule
  if(real_t)
    set(CONTENT "${CONTENT}\n  gROOT->ProcessLine(\"#define BDM_REALT ${real_t}\")\;")
  endif()
  if (dict)
    set(CONTENT "${CONTENT}\n  gROOT->ProcessLine(\"#define USE_DICT\")\;")
    set(CONTENT "${CONTENT}\n  gROOT->ProcessLine(\"R__ADD_INCLUDE_PATH($BDMSYS/include)\")\;")
    set(CONTENT "${CONTENT}\n  gROOT->ProcessLine(\"R__ADD_LIBRARY_PATH($BDMSYS/lib)\")\;")
    set(CONTENT "${CONTENT}\n  gROOT->ProcessLine(\"R__LOAD_LIBRARY(libbiodynamo)\")\;")
    # We add this one because the ROOT visualization require it, and it's not one
    # of the core libraries that is loaded by default in rootcling
    set(CONTENT "${CONTENT}\n  gROOT->ProcessLine(\"R__LOAD_LIBRARY(GenVector)\")\;")
    set(CONTENT "${CONTENT}\n  gROOT->ProcessLine(\"#include \\\"biodynamo.h\\\"\")\;")
    set(CONTENT "${CONTENT}\n  gROOT->ProcessLine(\"using namespace bdm\;\")\;")
    set(CONTENT "${CONTENT}\n  gROOT->ProcessLine(\"Simulation simulation(\\\"simulation\\\")\;\")\;")
    set(CONTENT "${CONTENT}\n  gROOT->ProcessLine(\"cout << \\\"INFO: Created simulation object 'simulation' with UniqueName='simulation'.\\\" << endl\;\")\;")
  else()
    set(CONTENT "${CONTENT}\n  gROOT->ProcessLine(\"cout << \\\"ERROR: Loading BioDynaMo into ROOT failed!\\\" << endl\;\")\;")
    set(CONTENT "${CONTENT}\n  gROOT->ProcessLine(\"cout << \\\"       BioDynaMo was not built with dict=ON\\\" << endl\;\")\;")
  endif()
  
  set(CONTENT "${CONTENT}\n}\n")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/rootlogon.C" ${CONTENT})
endfunction(generate_rootlogon)

# Fix for rootcling not able to find omp.h. We cannot include the entire include
# directory of the compiler (where omp.h is in), because rootcling starts using
# the intrinsics found in there. Therefore we do a local copy into the build dir.
function(fix_rootcling_omp_issue)
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=include/omp.h
                  OUTPUT_VARIABLE OMP_HEADER_PATH)
  # above command returns path with "\n" appended
  string(REGEX REPLACE "\n$" "" OMP_HEADER_PATH "${OMP_HEADER_PATH}")

  # Get major GCC version
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
                  OUTPUT_VARIABLE GCC_VERSION)
  string(REGEX REPLACE "\n$" "" GCC_VERSION "${GCC_VERSION}")
  string(REGEX MATCH "^[0-9]+" GCC_VERSION_MAJOR "${GCC_VERSION}")

  # Use third_party/omp/omp.h if compiler didn't resolve full path or gcc version > 11
  if ("${OMP_HEADER_PATH}" STREQUAL "include/omp.h" OR GCC_VERSION_MAJOR GREATER 11)
    if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/omp/omp.h")
      execute_process(COMMAND cp -a ${CMAKE_SOURCE_DIR}/third_party/omp ${CMAKE_BINARY_DIR})
    elseif(EXISTS "$ENV{BDMSYS}/third_party/omp/omp.h")
      execute_process(COMMAND cp -a $ENV{BDMSYS}/third_party/omp ${CMAKE_BINARY_DIR})
    elseif(EXISTS "$ENV{BDMSYS}/omp/omp.h")
      execute_process(COMMAND cp -a $ENV{BDMSYS}/omp ${CMAKE_BINARY_DIR})
    else()
      message(FATAL_ERROR "The BioDynaMo environment is not set up correctly. Please execute 'source <path-to-bdm-installation>/bin/thisbdm.sh' and retry this command.")
    endif()
  else()
    execute_process(COMMAND mkdir -p ${CMAKE_BINARY_DIR}/omp)
    execute_process(COMMAND cp -f ${OMP_HEADER_PATH} ${CMAKE_BINARY_DIR}/omp)
  endif()
  include_directories("${CMAKE_BINARY_DIR}/omp")
endfunction(fix_rootcling_omp_issue)

# Fix to be able to find opencl C++ include file cl2.hpp. On macOS this include
# is missing so we should make sure it is available during compilation on macOS.
function(fix_macos_opencl_header_issue)

  if (APPLE)
     if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/opencl/cl2.hpp")
       execute_process(COMMAND cp -a ${CMAKE_SOURCE_DIR}/third_party/opencl ${CMAKE_BINARY_DIR})
     elseif(EXISTS "$ENV{BDMSYS}/third_party/opencl/cl2.hpp")
       execute_process(COMMAND cp -a $ENV{BDMSYS}/third_party/opencl ${CMAKE_BINARY_DIR})
     elseif(EXISTS "$ENV{BDMSYS}/opencl/cl2.hpp")
       execute_process(COMMAND cp -a $ENV{BDMSYS}/opencl ${CMAKE_BINARY_DIR})
     else()
       message(FATAL_ERROR "The BioDynaMo environment is not set up correctly. Please execute 'source <path-to-bdm-installation>/bin/thisbdm.sh' and retry this command.")
     endif()
     include_directories("${CMAKE_BINARY_DIR}/opencl")
  endif()

endfunction(fix_macos_opencl_header_issue)
