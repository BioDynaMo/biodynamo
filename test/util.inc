#!/bin/bash
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

# Compiles and runs the CMake-based simulation code contained in $1.
#
#   $1 the directory containing the CMake simulation code
function run_cmake_simulation() {
  if [ -n "$ZSH_VERSION" ]; then
    emulate -LR bash
  fi
  log=$(mktemp)
  echo "created logfile ${log}"

  if [ -n "$ZSH_VERSION" ]; then
    TRAPEXIT() {
      rm -rf "${log}"
      sleep 1
    }
  else
    trap "rm -rf \"${log}\"" RETURN
  fi

  pushd "$1"
  biodynamo run | tee "${log}"
  SIM_RET_VAL=$?
  popd

  if [ "${SIM_RET_VAL}" != "0" ]; then
    return $SIM_RET_VAL
  fi

  expected="Simulation completed successfully!"
  grep --quiet "${expected}" "${log}"
  return $?
}

if [ -n "$BASH_VERSION" ]; then
  BDM_PROJECT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
elif [ -n "$ZSH_VERSION" ]; then
  # shellcheck disable=SC2154
  BDM_PROJECT_DIR=$(cd "$(dirname "${(%):-%x}")/.." && pwd)
fi

CMAKE_DEMOS=$(find "${BDM_PROJECT_DIR}/demo" -name CMakeLists.txt \
                -exec sh -c 'basename $(dirname {})' \;)
