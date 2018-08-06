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

# setup google test
ExternalProject_Add(
  gtest
  URL "${CMAKE_SOURCE_DIR}/third_party/gtest-1.7.0.zip"
  PREFIX "${CMAKE_CURRENT_BINARY_DIR}/gtest"
  CMAKE_CACHE_ARGS
    -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
  INSTALL_COMMAND "" # Disable install step
  # Ugly but necessary, in future versions one can use ${binary_dir}
  # in BUILD_BYPRODUCTS
  #BUILD_BYPRODUCTS "${binary_dir}/libgtest.a"
  BUILD_BYPRODUCTS "${CMAKE_BINARY_DIR}/gtest/src/gtest-build/libgtest.a"
)
ExternalProject_Get_Property(gtest source_dir binary_dir)

# Create a libgtest target to be used as a dependency by test program
add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)
set_target_properties(libgtest PROPERTIES
    IMPORTED_LOCATION "${binary_dir}/libgtest.a"
    IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
)

# add include directories for gtest
include_directories("${CMAKE_BINARY_DIR}/gtest/src/gtest/include")

# create target that shows the test ouput on failure
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure)

# add custom clean target for test project
add_custom_target(testbdmclean)
add_dependencies(bdmclean testbdmclean)

if (coverage)
  include(CodeCoverage)
  # add_custom_target(coverage)
  SETUP_TARGET_FOR_COVERAGE("coverage" "${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target check" coverage)
endif()
# create coverage report in separate directory
# since building the coverage report requires different compiler flags building
# it in a separate directory keeps the current build directory in good order
add_custom_target(coverage-build
  COMMAND "${CMAKE_SOURCE_DIR}/util/housekeeping/create-coverage-report.sh" ${PROJECT_SOURCE_DIR} ${CMAKE_BINARY_DIR}
  COMMENT "Generate coverage report in separate directory
     Open the following file in your browser: ${CMAKE_BINARY_DIR}/coverage/coverage/index.html")


function(bdm_add_test_executable TEST_TARGET)
  cmake_parse_arguments(ARG "" "" "SOURCES;HEADERS" ${ARGN} )
  # create test executable
  bdm_add_executable(${TEST_TARGET}
                     SOURCES ${ARG_SOURCES}
                     HEADERS ${ARG_HEADERS}
                     LIBRARIES libgtest biodynamo)
  add_dependencies(${TEST_TARGET}-objectlib gtest)

  # execute all tests with command: make test
  add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})

  # add valgrind test
  if (valgrind AND NOT coverage AND NOT ${TEST_TARGET} STREQUAL "runBiodynamoTests_mechanical_interaction_test")
    # filter out SchedulerTest.Backup because of timing issue 
    add_test(NAME "valgrind_${TEST_TARGET}" COMMAND valgrind --track-origins=yes --leak-resolution=high --tool=memcheck --leak-check=full --show-leak-kinds=all --gen-suppressions=all --show-reachable=no --suppressions=${CMAKE_BINARY_DIR}/../util/valgrind-bdm.supp --error-exitcode=1 ./${TEST_TARGET} -- --gtest_filter=-*DeathTest.*:IOTest.InvalidRead:SchedulerTest.Backup)
  endif()

  add_dependencies(check ${TEST_TARGET})

  add_custom_target("testbdmclean_${TEST_TARGET}" COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/CMakeFiles/${TEST_TARGET}.dir/cmake_clean.cmake")
  add_dependencies(testbdmclean "testbdmclean_${TEST_TARGET}")

endfunction(bdm_add_test_executable)
