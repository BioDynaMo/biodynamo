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

if [ $# -ne 1 ]; then
  echo "Wrong number of arguments.
Description:
  Run installation tests
Usage:
  installation_test.sh BDM_PROJECT_DIR
Arguments:
  BDM_PROJECT_DIR path to the biodynamo project directory
  "
  exit 1
fi

set -e -x

BDM_PROJECT_DIR=$1
cd $BDM_PROJECT_DIR

# speed-up build by disabling tests and demos
export BDM_CMAKE_FLAGS="-Dtest=off -Ddemo=off"

# Currently we are inside the biodynamo project directory, mapped as volume
# from the host
./install.sh . << EOF
y
EOF

# reload shell and source biodynamo
set +e +x
. $BDM_PROJECT_DIR/util/installation/common/util.sh
. $(BashrcFile)
$use_biodynamo
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
