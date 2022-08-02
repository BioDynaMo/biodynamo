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

# setup google test
ExternalProject_Add(
  bdm-gtest
  URL "${CMAKE_SOURCE_DIR}/third_party/gtest-1.11.0.zip"
  PREFIX "${CMAKE_CURRENT_BINARY_DIR}/gtest"
  CMAKE_ARGS
    -DPYTHON_EXECUTABLE=${Python_EXECUTABLE}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    -DCMAKE_VISIBILITY_INLINES_HIDDEN:BOOL=ON
    -DCMAKE_POLICY_DEFAULT_CMP0063=NEW
  CMAKE_CACHE_ARGS
    -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
    INSTALL_COMMAND 
     cp -a "${CMAKE_CURRENT_BINARY_DIR}/gtest/src/bdm-gtest/googletest/include/."
        "${CMAKE_CURRENT_BINARY_DIR}/include"
     && cp "${CMAKE_CURRENT_BINARY_DIR}/gtest/src/bdm-gtest-build/lib/libgtest.a" 
        "${CMAKE_CURRENT_BINARY_DIR}/lib"
  # Ugly but necessary, in future versions one can use ${binary_dir}
  # in BUILD_BYPRODUCTS
  #BUILD_BYPRODUCTS "${binary_dir}/libgtest.a"
  BUILD_BYPRODUCTS "${CMAKE_BINARY_DIR}/gtest/src/bdm-gtest-build/lib/libgtest.a"
)
ExternalProject_Get_Property(bdm-gtest source_dir binary_dir)

# Create a libgtest target to be used as a dependency by test program
add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest bdm-gtest)
set_target_properties(libgtest PROPERTIES
    IMPORTED_LOCATION "${binary_dir}/lib/libgtest.a"
    IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
)

# add include directories for gtest
include_directories("${CMAKE_BINARY_DIR}/gtest/src/bdm-gtest/googletest/include")

# create target that shows the test output on failure
add_custom_target(run-check COMMAND ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure)

# create target for running biodynamo-unit-tests
add_custom_target(run-unit-tests COMMAND ${CMAKE_BINARY_DIR}/bin/biodynamo-unit-tests)
add_dependencies(run-unit-tests biodynamo-unit-tests)

# add custom clean target for test project
add_custom_target(testbdmclean)
add_dependencies(bdmclean testbdmclean)

if (coverage)
  find_program(KCOV_PATH kcov REQUIRED)
  add_custom_target(coverage
    COMMAND ${CMAKE_BINARY_DIR}/launcher.sh ${KCOV_PATH} --include-path="${PROJECT_SOURCE_DIR}/src" coverage bin/biodynamo-unit-tests --gtest_filter=-*DeathTest*
  )
  # Add a custom target that is called in the sonar-source GHA to report coverage information in the dashboard.
  # The target ignores more test cases than the standard coverage target.
  add_custom_target(coverage-gha
    COMMAND ${CMAKE_BINARY_DIR}/launcher.sh ${KCOV_PATH} --include-path="${PROJECT_SOURCE_DIR}/src" coverage bin/biodynamo-unit-tests --gtest_filter=-*DeathTest*:*ExportToFile*:*GenerateSimulationInfoJson*:*Paraview*
  )
endif()
# create coverage report in separate directory
# since building the coverage report requires different compiler flags building
# it in a separate directory keeps the current build directory in good order
add_custom_target(coverage-build
  COMMAND "${CMAKE_SOURCE_DIR}/util/housekeeping/create-coverage-report.sh" ${PROJECT_SOURCE_DIR} ${CMAKE_BINARY_DIR}
  COMMENT "Generate coverage report in separate directory
     Open the following file in your browser: ${CMAKE_BINARY_DIR}/coverage/coverage/index.html"
)


function(bdm_add_test_executable TEST_TARGET)
  cmake_parse_arguments(ARG "" "" "SOURCES;HEADERS;LIBRARIES;NUM_MPI_RANKS;VALGRIND_TEST_FILTER" ${ARGN} )
  # create test executable
  bdm_add_executable(${TEST_TARGET}
                     SOURCES ${ARG_SOURCES}
                     HEADERS ${ARG_HEADERS}
                     LIBRARIES biodynamo libgtest ${ARG_LIBRARIES})
  set(BDM_MPIRUN "")
  if (${ARG_NUM_MPI_RANKS} GREATER 1)
    set(BDM_MPIRUN "mpirun -n ${ARG_NUM_MPI_RANKS}")
  endif()
  # execute all tests with command: make test
  add_test(NAME ${TEST_TARGET} COMMAND ${CMAKE_BINARY_DIR}/launcher.sh ${BDM_MPIRUN} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_TARGET})

  # add valgrind test
  if (valgrind AND VALGRIND_FOUND AND NOT coverage)
    # filter out tests that would take too long if tested under valgrind 
    add_test(NAME "valgrind_${TEST_TARGET}"
      COMMAND  ${CMAKE_BINARY_DIR}/launcher.sh ${BDM_MPIRUN} ${CMAKE_SOURCE_DIR}/util/valgrind.sh ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_TARGET} -- --gtest_filter=${ARG_VALGRIND_TEST_FILTER})
    add_custom_target(run-valgrind_${TEST_TARGET}
      COMMAND  ${CMAKE_BINARY_DIR}/launcher.sh ${BDM_MPIRUN} ${CMAKE_SOURCE_DIR}/util/valgrind.sh ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_TARGET} -- --gtest_filter=${ARG_VALGRIND_TEST_FILTER})
    add_dependencies(run-valgrind_${TEST_TARGET} biodynamo-unit-tests)

    if (NOT TARGET run-valgrind)
      add_custom_target(run-valgrind)
    endif()
    add_dependencies(run-valgrind run-valgrind_${TEST_TARGET})
  endif()

  add_dependencies(run-check ${TEST_TARGET})

  add_custom_target("testbdmclean_${TEST_TARGET}" COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/CMakeFiles/${TEST_TARGET}.dir/cmake_clean.cmake")
  add_dependencies(testbdmclean "testbdmclean_${TEST_TARGET}")

endfunction(bdm_add_test_executable)
