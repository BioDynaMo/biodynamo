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
  Run installation tests
Usage:
  installation-test.sh
No Arguments
  "
  exit 1
fi

set -e -x

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/.."
cd $BDM_PROJECT_DIR
# Currently we are inside the biodynamo project directory, mapped as volume
# from the host

# speed-up build by disabling tests and demos
export BDM_CMAKE_FLAGS="-Dtest=off -Ddemo=off"
./install.sh << EOF
y
y
EOF

# reload shell and source biodynamo
set +e +x
source ~/.bdm/biodynamo-env.sh
set -e -x

# Run all the demos.
DEMOS=(
    cell_division
    diffusion
    gene_regulation
    multiple_simulations
)
for demo_name in "${DEMOS[@]}"
do
  demo_dir=$(mktemp -d)
  biodynamo demo "${demo_name}" "${demo_dir}"
  run_simulation "${demo_dir}/${demo_name}"
  rm -rf "${demo_dir}"
done

# makefile_project
"${BDM_PROJECT_DIR}/test/integration/makefile_project.sh"

# verify if out of source builds work
cd ~
biodynamo new test-sim --no-github
run_simulation test-sim

exit $?
