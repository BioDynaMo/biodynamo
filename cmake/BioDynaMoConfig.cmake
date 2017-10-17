# BioDynaMo configuration file for external projects
#
# Script defines the following variables
#   BioDynaMo_FOUND      - set to true of BioDynaMo has been found
#   BioDynaMo_USE_FILE   - cmake script that will setup the build
#
# If an external project can't locate this file emitting the following error
# message:
#      Could not find a package configuration file provided by "BioDynaMo" with
#        any of the following names:
#
#          BioDynaMoConfig.cmake
#          biodynamo-config.cmake
#
# make sure the folder containing this file is in one of the folders described
# in https://cmake.org/cmake/help/latest/command/find_package.html where
# '<prefix>' are all directories inside  PATH environment variable.
# Use`cmake -DCMAKE_FIND_DEBUG_MODE=ON ..` to debug the issue.

find_file(BioDynaMo_USE_FILE
          UseBioDynaMo.cmake
          PATHS "/usr/share/biodynamo/cmake"
          		"/usr/local/share/cmake"
          ENV "BDM_CMAKE_DIR")

if(NOT BioDynaMo_USE_FILE AND BioDynaMo_FIND_REQUIRED)
  message(FATAL_ERROR "BioDynaMo not found! Try to set BDM_CMAKE_DIR environment variable \
                       pointing to the directory that contains the file UseBioDynaMo.cmake \
                       or add -DBDM_CMAKE_DIR=... to the cmake command")
elseif(NOT BioDynaMo_USE_FILE AND NOT BioDynaMo_FIND_QUIETLY)
  message(WARNING "BioDynaMo not found! Try to set BDM_CMAKE_DIR environment variable \
                   pointing to the directory that contains the file UseBioDynaMo.cmake \
                   or add -DBDM_CMAKE_DIR=... to the cmake command")
endif()

set(BioDynaMo_FOUND TRUE)
