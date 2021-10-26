#!/bin/bash
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

# Compiles and runs the CMake-based simulation code contained in $1.
#
#   $1 the directory containing the CMake simulation code
function run_cmake_simulation() {
  # Create a temporary file on your system referenced by log 
  log=$(mktemp)
  # Set trap, e.g. whenever the scirpt returns, we delete the temporary file
  trap "rm -rf \"${log}\"" RETURN
  # Check if we run a test simulation (system test) or regular demo
  ISTEST=false
  if [ "$1" = "test-sim" ]; then
    ISTEST=true
  fi

  # Navigate to folder provided to the script `run_cmake_simulation <folder>`
  pushd "$1"
  # Execute BioDynaMo simlation and save output to temporary file
  biodynamo run | tee "${log}"
  # Get return value of simulation (zero if successful)
  SIM_RET_VAL=$?

  # If we are running the system tests with template `test-sim`, we also test
  # the `biodynamo test` command
  if [ "$ISTEST" = "true" ]; then
    # We test both ways because biodynamo test returns the ctest return value.
    # The return value is 0 even if a test fails. This is not the case for 
    # the second option.
    biodynamo test 
    SIM_RET_VAL+=$?
    ./build/test-sim-test
    SIM_RET_VAL+=$?
  fi

  # return to original dictionary
  popd

  # Check if run and test commands were successful
  if [[ "$ISTEST" == true && "${SIM_RET_VAL}" != "000" ]]; then
    echo -n "Faild with return value ${SIM_RET_VAL}. The three digits "
    echo -n "correspond to:\nSimulation return value, ctest return value, test"
    echo " results."
    return 1 
  elif [ "${SIM_RET_VAL}" != "0" ]; then
    return $SIM_RET_VAL
  fi

  # If commands were successful, we further check if the output of the 
  # simulation is correct.
  expected="Simulation completed successfully!"
  grep --quiet "${expected}" "${log}"
  return $?
}

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CMAKE_DEMOS=$(find "${BDM_PROJECT_DIR}/demo" -name CMakeLists.txt \
                -exec sh -c 'basename $(dirname {})' \;)
