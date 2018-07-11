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
  log=$(mktemp)
  trap "rm -rf \"${log}\"" RETURN

  pushd "$1"
  biodynamo run | tee "${log}"
  popd

  expected="Simulation completed successfully!"
  actual=$(tail -n3 "${log}" | head -n1)

  if [ "${actual}" != "${expected}" ]; then
    return 1
  fi

  return 0
}

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CMAKE_DEMOS=$(find "${BDM_PROJECT_DIR}/demo" -name CMakeLists.txt \
                -exec sh -c 'basename $(dirname {})' \;)
