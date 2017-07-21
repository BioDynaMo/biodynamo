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
#                              LIBRARIES lib1 lib2 ...)
# BioDynaMo's version of add_executable
# This is required because ROOT dictionaries must be built beforehand.
# \param TARGET target name for the executable
# \param SOURCES list of source files
# \param LIBRARIES list of libraries that should be linked to the executable.
#        can also be a target name of a library
function(bdm_add_executable TARGET)
  cmake_parse_arguments(ARG "" "" "SOURCES;LIBRARIES" ${ARGN} )

  get_implicit_dependencies(INCLUDED_HEADERS SOURCES ${ARG_SOURCES})
  # generate dictionaries using genreflex
  if(INCLUDED_HEADERS)
    # dictionary
    set(DICT_FILE_WE ${TARGET}_dict)
    REFLEX_GENERATE_DICTIONARY("${DICT_FILE_WE}" ${INCLUDED_HEADERS}
                               SELECTION cmake/selection.xml)
    # dictionary with custom streamers
    set(DICT_FILE_CS_WE ${TARGET}_custom_streamers_dict)
    REFLEX_GENERATE_DICTIONARY("${DICT_FILE_CS_WE}" ${INCLUDED_HEADERS}
                               SELECTION cmake/selection_custom_streamers.xml)
    set(DICT_SRCS ${DICT_FILE_WE}.cc ${DICT_FILE_CS_WE}.cc)
  endif()

  # generate executable
  add_executable(${TARGET} ${DICT_SRCS} ${ARG_SOURCES})
  target_link_libraries(${TARGET} ${ARG_LIBRARIES})

endfunction(bdm_add_executable)

# function generate_rootlogon
# generates rootlogon.C which is required by ROOT's C++ interpreter cling
function(generate_rootlogon)
  set(CONTENT "{\n")
  get_directory_property(INCLUDE_DIRS INCLUDE_DIRECTORIES)
  set(INCLUDE_OPTIONS)
  foreach( DIR ${INCLUDE_DIRS})
    set(CONTENT "${CONTENT}\n  gROOT->ProcessLine(\".include ${DIR}\")\;")
  endforeach()
  set(CONTENT "${CONTENT}\n  gROOT->ProcessLine(\".L libbiodynamo.so\")\;\n}")

  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/rootlogon.C" ${CONTENT})
endfunction(generate_rootlogon)
