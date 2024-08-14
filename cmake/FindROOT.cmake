# - Finds ROOT installation
# This module sets up ROOT information
# It defines:
# ROOT_FOUND             If the ROOT is found
# ROOT_INCLUDE_DIR       PATH to the include directory
# ROOT_INCLUDE_DIRS      PATH to the include directories (not cached)
# ROOT_LIBRARIES         Most common libraries
# ROOT_<name>_LIBRARY    Full path to the library <name>
# ROOT_LIBRARY_DIR       PATH to the library directory
# ROOT_ETC_DIR           PATH to the etc directory
# ROOT_DEFINITIONS       Compiler definitions
# ROOT_CXX_FLAGS         Compiler flags to be used by client packages
# ROOT_C_FLAGS           Compiler flags to be used by client packages
# ROOT_EXE_LINKER_FLAGS  Linker flags to be used by client packages
#
# Updated by K. Smith (ksmith37@nd.edu) to properly handle
#  dependencies in ROOT_GENERATE_DICTIONARY

find_program(ROOT_CONFIG_EXECUTABLE NAMES root-config
  HINTS "$ENV{ROOTSYS}/bin" "$ENV{BDM_ROOT_DIR}/bin" "${CMAKE_THIRD_PARTY_DIR}/root/bin")

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --prefix
    OUTPUT_VARIABLE ROOTSYS
    OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --version
    OUTPUT_VARIABLE ROOT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --incdir
    OUTPUT_VARIABLE ROOT_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)
set(ROOT_INCLUDE_DIRS ${ROOT_INCLUDE_DIR})

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --etcdir
    OUTPUT_VARIABLE ROOT_ETC_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)
set(ROOT_ETC_DIRS ${ROOT_ETC_DIR})

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --libdir
    OUTPUT_VARIABLE ROOT_LIBRARY_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)
set(ROOT_LIBRARY_DIRS ${ROOT_LIBRARY_DIR})

set(rootlibs Core RIO Net Hist Graf Graf3d Gpad Tree Rint Postscript Matrix Physics MathCore Thread MultiProc Imt)
set(ROOT_LIBRARIES)
foreach(_cpt ${rootlibs} ${ROOT_FIND_COMPONENTS})
    find_library(ROOT_${_cpt}_LIBRARY ${_cpt} HINTS ${ROOT_LIBRARY_DIR})
  if(ROOT_${_cpt}_LIBRARY)
    mark_as_advanced(ROOT_${_cpt}_LIBRARY)
    list(APPEND ROOT_LIBRARIES ${ROOT_${_cpt}_LIBRARY})
    if(ROOT_FIND_COMPONENTS)
      list(REMOVE_ITEM ROOT_FIND_COMPONENTS ${_cpt})
    endif()
  endif()
endforeach()
if(ROOT_LIBRARIES)
  list(REMOVE_DUPLICATES ROOT_LIBRARIES)
endif()

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --cflags
    OUTPUT_VARIABLE __cflags
    OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX MATCHALL "-(D|U)[^ ]*" ROOT_DEFINITIONS "${__cflags}")
string(REGEX REPLACE "(^|[ ]*)-I[^ ]*" "" ROOT_CXX_FLAGS "${__cflags}")
string(REGEX REPLACE "(^|[ ]*)-I[^ ]*" "" ROOT_C_FLAGS "${__cflags}")

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --ldflags
    OUTPUT_VARIABLE __ldflags
    OUTPUT_STRIP_TRAILING_WHITESPACE)
set(ROOT_EXE_LINKER_FLAGS "${__ldflags}")

set(ROOT_USE_FILE ${CMAKE_CURRENT_LIST_DIR}/RootUseFile.cmake)

execute_process(
  COMMAND ${ROOT_CONFIG_EXECUTABLE} --features
  OUTPUT_VARIABLE _root_options
  OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(_root_options)
foreach(_opt ${_root_options})
  set(ROOT_${_opt}_FOUND TRUE)
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ROOT DEFAULT_MSG ROOT_CONFIG_EXECUTABLE
    ROOTSYS ROOT_VERSION ROOT_INCLUDE_DIR ROOT_LIBRARIES ROOT_LIBRARY_DIR)

mark_as_advanced(ROOT_CONFIG_EXECUTABLE)

include(CMakeParseArguments)
find_program(ROOTCLING_EXECUTABLE rootcling
  HINTS "$ENV{ROOTSYS}/bin" "$ENV{BDM_ROOT_DIR}/bin" "${CMAKE_THIRD_PARTY_DIR}/root/bin")
#find_package(GCCXML)

# We use the launcher script to emulate a `source thisbdm.sh` call
if(NOT BDM_OUT_OF_SOURCE)
  set(LAUNCHER ${CMAKE_BINARY_DIR}/launcher.sh)
endif()

include(${CMAKE_SOURCE_DIR}/cmake/RootMacros.cmake)