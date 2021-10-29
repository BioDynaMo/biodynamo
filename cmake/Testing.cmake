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
  gtest
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
     cp -a "${CMAKE_CURRENT_BINARY_DIR}/gtest/src/gtest/googletest/include/."
        "${CMAKE_CURRENT_BINARY_DIR}/include"
     && cp "${CMAKE_CURRENT_BINARY_DIR}/gtest/src/gtest-build/lib/libgtest.a" 
        "${CMAKE_CURRENT_BINARY_DIR}/lib"
  # Ugly but necessary, in future versions one can use ${binary_dir}
  # in BUILD_BYPRODUCTS
  #BUILD_BYPRODUCTS "${binary_dir}/libgtest.a"
  BUILD_BYPRODUCTS "${CMAKE_BINARY_DIR}/gtest/src/gtest-build/lib/libgtest.a"
)
ExternalProject_Get_Property(gtest source_dir binary_dir)

# Create a libgtest target to be used as a dependency by test program
add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)
set_target_properties(libgtest PROPERTIES
    IMPORTED_LOCATION "${binary_dir}/lib/libgtest.a"
    IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
)

# add include directories for gtest
include_directories("${CMAKE_BINARY_DIR}/gtest/src/gtest/googletest/include")

# create target that shows the test output on failure
add_custom_target(run-check COMMAND ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure)

# create target for running biodynamo-unit-tests
add_custom_target(run-unit-tests COMMAND ${CMAKE_BINARY_DIR}/bin/biodynamo-unit-tests)
add_dependencies(run-unit-tests biodynamo-unit-tests)

# add custom clean target for test project
add_custom_target(testbdmclean)
add_dependencies(bdmclean testbdmclean)

if (coverage)
  find_program(KCOV_PATH kcov)
  add_custom_target(coverage
    COMMAND ${CMAKE_BINARY_DIR}/launcher.sh ${KCOV_PATH} --include-path="${PROJECT_SOURCE_DIR}/src" coverage bin/biodynamo-unit-tests --gtest_filter=-*DeathTest*
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
  cmake_parse_arguments(ARG "" "" "SOURCES;HEADERS;LIBRARIES" ${ARGN} )
  # create test executable
  bdm_add_executable(${TEST_TARGET}
                     SOURCES ${ARG_SOURCES}
                     HEADERS ${ARG_HEADERS}
                     LIBRARIES biodynamo libgtest ${ARG_LIBRARIES})
  SET(BIODYNAMO_TEST_TARGET_NAME "${TEST_TARGET}" PARENT_SCOPE)

  # execute all tests with command: make test
  add_test(NAME ${TEST_TARGET} COMMAND ${CMAKE_BINARY_DIR}/launcher.sh ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_TARGET})

  # add valgrind test
  if (valgrind AND VALGRIND_FOUND AND NOT coverage)
    # filter out tests that would take too long if tested under valgrind 
    add_test(NAME "valgrind_${TEST_TARGET}"
      COMMAND  ${CMAKE_BINARY_DIR}/launcher.sh ${CMAKE_SOURCE_DIR}/util/valgrind.sh ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_TARGET} -- --gtest_filter=-*DeathTest.*:IOTest.InvalidRead:SchedulerTest.Backup:ResourceManagerTest.SortAndForEachAgentParallel*:InlineVector*:NeuriteElementBehaviour.*:MechanicalInteraction.*:DiffusionTest.*Convergence*:FLAKY_ParaviewIntegrationTest*:AgentVectorTest.Equality:SchedulerTest::LoadAndBalanceAfterEnvironment)
    add_custom_target(run-valgrind
      COMMAND  ${CMAKE_BINARY_DIR}/launcher.sh ${CMAKE_SOURCE_DIR}/util/valgrind.sh ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_TARGET} -- --gtest_filter=-*DeathTest.*:IOTest.InvalidRead:SchedulerTest.Backup:ResourceManagerTest.SortAndForEachAgentParallel*:InlineVector*:NeuriteElementBehaviour.*:MechanicalInteraction.*:DiffusionTest.*Convergence*:FLAKY_ParaviewIntegrationTest*:AgentVectorTest.Equality:SchedulerTest::LoadAndBalanceAfterEnvironment)
    add_dependencies(run-valgrind biodynamo-unit-tests)
  endif()

  add_dependencies(run-check ${TEST_TARGET})

  # add target for system tests
  add_custom_target(run-demos COMMAND ${CMAKE_SOURCE_DIR}/test/system-test.sh)

  add_custom_target("testbdmclean_${TEST_TARGET}" COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/CMakeFiles/${TEST_TARGET}.dir/cmake_clean.cmake")
  add_dependencies(testbdmclean "testbdmclean_${TEST_TARGET}")

endfunction(bdm_add_test_executable)
