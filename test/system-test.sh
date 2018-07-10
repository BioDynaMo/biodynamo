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

# Compiles and runs the simulation code contained in $1.
#
#   $1 the directory containing the simulation code
function run_simulation() {
  pushd "$1"
  log=$(mktemp)
  expected="Simulation completed successfully!"
  biodynamo run | tee "${log}"
  actual=$(tail -n3 "${log}" | head -n1)
  popd

  if [ "${actual}" != "${expected}" ]; then
    exit 1
  fi

  rm -rf "${log}"
}

if [ $# -ne 0 ]; then
  echo "Wrong number of arguments.
Description:
  Run system tests in a BioDynaMo environment.
Usage:
  system-test.sh
No Arguments
  "
  exit 1
fi

set -e -x

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/.."

# Run all the CMake demos.
DEMOS=$(find "${BDM_PROJECT_DIR}/demo" -name CMakeLists.txt \
          -exec sh -c 'basename $(dirname {})' \;)
for demo_name in ${DEMOS[@]}  # No quotation.
do
  demo_dir=$(mktemp -d)
  biodynamo demo "${demo_name}" "${demo_dir}"
  run_simulation "${demo_dir}/${demo_name}"
  rm -rf "${demo_dir}"
done

# makefile_project
"${BDM_PROJECT_DIR}/test/integration/makefile_project.sh"

exit $?
