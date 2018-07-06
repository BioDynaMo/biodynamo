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

# verify if out of source builds work
cd ~
biodynamo new test-sim --no-github
cd test-sim
biodynamo run &>all

# extract ouput
cat all | tail -n3 | head -n1 >actual

# create file with expected output
echo "Simulation completed successfully!" >> expected

diff expected actual
exit $?
